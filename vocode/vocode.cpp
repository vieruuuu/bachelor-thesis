#include "vocode.hpp"

std::vector<double> vocode(const std::vector<double> &audio,
                           const std::vector<double> &original_f0,
                           const std::vector<double> &target_f0,
                           int sample_rate, int hop_length,
                           double fade_duration_ms) {
  size_t num_samples = audio.size();
  size_t num_frames = original_f0.size();

  // Edge case: empty F0 contours
  if (num_frames == 0) {
    std::cerr << "Warning: F0 contours are empty. Returning original audio.\n";
    return audio;
  }

  // 1. Find analysis marks (epochs) in original audio
  std::vector<int> analysis_marks;
  double last_mark_time = 0.0;
  bool in_voiced_segment = false;

  for (size_t n = 0; n < num_frames; ++n) {
    double f0_val = original_f0[n];
    bool is_voiced = !std::isnan(f0_val) && f0_val > 0.0;
    if (is_voiced) {
      double period_samp = sample_rate / f0_val;
      double frame_center_time =
          (n * hop_length + hop_length / 2.0) / sample_rate;
      double current_time;

      if (!in_voiced_segment) {
        current_time = std::max(frame_center_time, last_mark_time);
        in_voiced_segment = true;
      } else {
        current_time = last_mark_time + period_samp / sample_rate;
      }

      double frame_end_time =
          (n + 1) * hop_length / static_cast<double>(sample_rate);
      double min_spacing = 0.5 * period_samp / sample_rate;

      while (current_time < frame_end_time &&
             current_time * sample_rate < num_samples) {
        if (current_time > last_mark_time + min_spacing) {
          int mark = static_cast<int>(std::round(current_time * sample_rate));
          analysis_marks.push_back(mark);
          last_mark_time = current_time;
          current_time += period_samp / sample_rate;
        } else {
          current_time += min_spacing;
        }
      }
    } else {
      in_voiced_segment = false;
    }
  }

  if (analysis_marks.empty()) {
    std::cerr << "Warning: No voiced frames found. Returning original audio.\n";
    return audio;
  }

  // Clip out-of-bounds marks
  analysis_marks.erase(
      std::remove_if(analysis_marks.begin(), analysis_marks.end(),
                     [&](int m) {
                       return m < 0 || static_cast<size_t>(m) >= num_samples;
                     }),
      analysis_marks.end());
  if (analysis_marks.empty()) {
    std::cerr << "Warning: All analysis marks out of bounds. Returning "
                 "original audio.\n";
    return audio;
  }

  // 2. Synthesis via TD-PSOLA
  std::vector<double> voiced_output(num_samples, 0.0);
  double synth_time = static_cast<double>(analysis_marks[0]);
  size_t last_ana_idx = 0;

  while (synth_time < static_cast<double>(num_samples)) {
    size_t frame_idx =
        std::min(static_cast<size_t>(synth_time) / hop_length, num_frames - 1);
    double t_f0 = target_f0[frame_idx];
    bool target_voiced = !std::isnan(t_f0) && t_f0 > 0.0;

    if (!target_voiced) {
      synth_time += hop_length;
      continue;
    }

    double target_period = sample_rate / t_f0;
    // Search for closest analysis mark
    int radius = 10;
    size_t start = (last_ana_idx < radius) ? 0 : last_ana_idx - radius;
    size_t end = std::min(last_ana_idx + radius + 1, analysis_marks.size());

    size_t closest_idx = start;
    double min_diff = std::abs(analysis_marks[start] - synth_time);
    for (size_t i = start + 1; i < end; ++i) {
      double diff = std::abs(analysis_marks[i] - synth_time);
      if (diff < min_diff) {
        min_diff = diff;
        closest_idx = i;
      }
    }
    last_ana_idx = closest_idx;
    int t_analysis = analysis_marks[closest_idx];

    // Original period at analysis point
    size_t ana_frame =
        std::min(static_cast<size_t>(t_analysis) / hop_length, num_frames - 1);
    double o_f0 = original_f0[ana_frame];
    if (std::isnan(o_f0) || o_f0 <= 0.0) {
      synth_time += target_period;
      continue;
    }
    double orig_period = sample_rate / o_f0;

    int win_len = static_cast<int>(std::round(2 * orig_period));
    if (win_len < 2)
      win_len = 2;
    if (win_len % 2 != 0)
      ++win_len;

    int ana_start = t_analysis - win_len / 2;
    int ana_end = t_analysis + win_len / 2;

    int seg_s = std::max(0, ana_start);
    int seg_e = std::min<int>(num_samples, ana_end);
    int seg_len = seg_e - seg_s;
    if (seg_len <= 0) {
      synth_time += target_period;
      continue;
    }

    // Hanning window
    std::vector<double> win(win_len);
    for (int i = 0; i < win_len; ++i) {
      win[i] = 0.5 * (1 - std::cos(2 * M_PI * i / (win_len - 1)));
    }

    int win_offset = seg_s - ana_start;
    std::vector<double> segment(seg_len);
    for (int i = 0; i < seg_len; ++i) {
      segment[i] = audio[seg_s + i] * win[win_offset + i];
    }

    int out_center = static_cast<int>(std::round(synth_time));
    int out_start = out_center - win_len / 2;
    int out_end = out_start + win_len;

    int ola_s_out = std::max(0, out_start);
    int ola_e_out = std::min<int>(num_samples, out_end);
    int seg_off = ola_s_out - out_start;
    int overlap = ola_e_out - ola_s_out;

    for (int i = 0; i < overlap; ++i) {
      voiced_output[ola_s_out + i] += segment[seg_off + i];
    }

    synth_time += target_period;
  }

  // 3. Voiced/unvoiced mask
  std::vector<bool> voiced_mask(num_samples, false);
  for (size_t n = 0; n < num_frames; ++n) {
    bool orig_v = !std::isnan(original_f0[n]) && original_f0[n] > 0.0;
    bool tgt_v = !std::isnan(target_f0[n]) && target_f0[n] > 0.0;
    if (orig_v && tgt_v) {
      int start = n * hop_length;
      int end = std::min<int>((n + 1) * hop_length, num_samples);
      for (int i = start; i < end; ++i)
        voiced_mask[i] = true;
    }
  }

  // 4. Smooth transitions with cross-fading
  std::vector<double> smooth_mask(num_samples);
  for (size_t i = 0; i < num_samples; ++i)
    smooth_mask[i] = voiced_mask[i] ? 1.0 : 0.0;

  int fade_samps = static_cast<int>(fade_duration_ms / 1000.0 * sample_rate);
  fade_samps = std::max(2, fade_samps);

  // Compute transitions
  std::vector<int> diff(num_samples);
  diff[0] = voiced_mask[0] ? 1 : 0;
  for (size_t i = 1; i < num_samples; ++i)
    diff[i] = (voiced_mask[i] ? 1 : 0) - (voiced_mask[i - 1] ? 1 : 0);

  // Ramps
  std::vector<double> fade_in(fade_samps), fade_out(fade_samps);
  for (int i = 0; i < fade_samps; ++i) {
    fade_in[i] = static_cast<double>(i) / (fade_samps - 1);
    fade_out[i] = 1.0 - static_cast<double>(i) / (fade_samps - 1);
  }

  for (size_t idx = 0; idx < num_samples; ++idx) {
    if (diff[idx] == 1) { // onset
      int end = std::min<int>(idx + fade_samps, num_samples);
      for (int j = idx; j < end; ++j)
        smooth_mask[j] = fade_in[j - idx];
    } else if (diff[idx] == -1) { // offset
      int start = std::max<int>(0, idx - fade_samps);
      for (int j = start; j < static_cast<int>(idx); ++j)
        smooth_mask[j] = fade_out[j - start];
    }
  }

  // 5. Combine signals
  std::vector<double> output(num_samples);
  for (size_t i = 0; i < num_samples; ++i) {
    output[i] =
        smooth_mask[i] * voiced_output[i] + (1.0 - smooth_mask[i]) * audio[i];
  }

  return output;
}
