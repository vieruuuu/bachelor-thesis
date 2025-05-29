#include "pyin_helper.hpp"

void write_output(real_t observation_probs[2 * n_pitch_bins],
                  real_t voiced_prob,
                  stream<real_t, 2 * n_pitch_bins> &observation_probs_stream) {
  for (int i = 0; i < 2 * n_pitch_bins; ++i) {
#pragma HLS PIPELINE II = 1 rewind
    observation_probs_stream.write(observation_probs[i]);
  }
}

void localmin(real_t yin_frame[yin_frame_size],
              stream<bool, yin_frame_size> &is_trough_stream) {
  // Set first element based on comparison with second element
  is_trough_stream.write(yin_frame[0] < yin_frame[1]);

  for (size_t i = 1; i < yin_frame_size - 1; ++i) {
    bool is_trough_value =
        yin_frame[i - 1] > yin_frame[i] && yin_frame[i + 1] > yin_frame[i];

    is_trough_stream.write(is_trough_value);
  }

  is_trough_stream.write(false);
}

void copy_stream_to_buffer(
    stream<real_t, yin_frame_size> &yin_frame_stream,
    stream<real_t, yin_frame_size> &parabolic_shifts_stream,
    real_t yin_frame[yin_frame_size], real_t parabolic_shifts[yin_frame_size]) {
#pragma HLS DATAFLOW

  for (int i = 0; i < yin_frame_size; ++i) {
#pragma HLS PIPELINE II = 1 rewind

    real_t yin_frame_value = yin_frame_stream.read();

    yin_frame[i] = yin_frame_value;
  }

  for (int i = 0; i < yin_frame_size; ++i) {
#pragma HLS PIPELINE II = 1 rewind

    real_t parabolic_shifts_value = parabolic_shifts_stream.read();

    parabolic_shifts[i] = parabolic_shifts_value;
  }
}

void pyin_helper(stream<real_t, yin_frame_size> &yin_frame_stream,
                 stream<real_t, yin_frame_size> &parabolic_shifts_stream,
                 stream<real_t, 2 * n_pitch_bins> &observation_probs_stream) {

  stream<int, thresholds_size - 1> n_troughs;
  stream<real_t, yin_frame_size> trough_heights;
  stream<int, yin_frame_size> trough_index;
  stream<bool, yin_frame_size> is_trough;

  bool trough_thresholds[yin_frame_size][thresholds_size - 1] = {false};
  int trough_positions[yin_frame_size][thresholds_size - 1] = {0};
  real_t trough_prior[yin_frame_size][thresholds_size - 1] = {0.0};
  real_t probs[yin_frame_size] = {0.0};
  real_t observation_probs[2 * n_pitch_bins] = {0.0};
  real_t yin_frame[yin_frame_size];
  real_t parabolic_shifts[yin_frame_size];

  int global_min_index;
  real_t global_min_value;

  copy_stream_to_buffer(yin_frame_stream, parabolic_shifts_stream, yin_frame,
                        parabolic_shifts);

  // Find the troughs (local minima)
  localmin(yin_frame, is_trough);

  int last_trough_index = 0;

  for (size_t i = 0; i < yin_frame_size; ++i) {
    bool is_trough_value = is_trough.read();

    if (!is_trough_value) {
      continue;
    }

    // Get trough indices
    trough_index.write(i);

    // Get trough heights
    real_t trough_height = yin_frame[i];
    trough_heights.write(trough_height);

    // Find global minimum
    if (last_trough_index == 0) {
      global_min_index = 0;
      global_min_value = trough_height;
    } else if (trough_height < global_min_value) {
      global_min_value = trough_height;
      global_min_index = last_trough_index;
    }

    last_trough_index += 1;
  }

  const int trough_thresholds_size = last_trough_index;

  // Return empty observation if no troughs found
  if (trough_thresholds_size == 0) {
    return write_output(observation_probs, 0.0, observation_probs_stream);
  }

  // Find which troughs are below each threshold
  for (size_t i = 0; i < trough_thresholds_size; ++i) {
    real_t trough_height = trough_heights.read();

    for (size_t j = 1; j < thresholds_size; ++j) {
      trough_thresholds[i][j - 1] = trough_height < thresholds[j];
    }
  }

  // Define the prior over the troughs using Boltzmann distribution
  for (size_t j = 0; j < thresholds_size - 1; ++j) {
    int col_sum = 0;
    for (size_t i = 0; i < trough_thresholds_size; ++i) {
      col_sum += trough_thresholds[i][j] ? 1 : 0;
    }

    if (col_sum > 0) {
      int cum_sum = 0;
      for (size_t i = 0; i < trough_thresholds_size; ++i) {
        if (trough_thresholds[i][j]) {
          trough_positions[i][j] = cum_sum;

          cum_sum++;
        }
      }
    }
  }

  // Count non-zero elements in each column

  for (size_t j = 0; j < thresholds_size - 1; ++j) {
    int trough_count = 0;

    for (size_t i = 0; i < trough_thresholds_size; ++i) {
      if (trough_thresholds[i][j]) {
        trough_count += 1;
      }
    }

    n_troughs.write(trough_count);
  }

  // Calculate trough prior using Boltzmann distribution

  for (size_t j = 0; j < thresholds_size - 1; ++j) {
    int trough_count = n_troughs.read();

    if (trough_count > 0) {
      for (size_t i = 0; i < trough_thresholds_size; ++i) {
        if (trough_thresholds[i][j]) {
          trough_prior[i][j] = boltzmann_pmf(trough_positions[i][j],
                                             boltzmann_parameter, trough_count);
        }
      }
    }
  }

  // For each threshold add probability to global minimum if no trough is below
  // threshold, else add probability to each trough below threshold biased by
  // prior
  for (size_t i = 0; i < trough_thresholds_size; ++i) {
    for (size_t j = 0; j < thresholds_size - 1; ++j) {
      probs[i] += trough_prior[i][j] * beta_probs[j];
    }
  }

  if (trough_thresholds_size > 0) {
    // Count thresholds below minimum
    int n_thresholds_below_min = 0;
    for (size_t j = 0; j < thresholds_size - 1; ++j) {
      if (!trough_thresholds[global_min_index][j]) {
        n_thresholds_below_min++;
      }
    }

    // Sum beta probabilities for thresholds below minimum
    real_t beta_sum = 0.0;
    for (int j = 0; j < n_thresholds_below_min; ++j) {
      beta_sum += beta_probs[j];
    }

    probs[global_min_index] += no_trough_prob * beta_sum;
  }

  for (size_t i = 0; i < trough_thresholds_size; ++i) {
    int idx = trough_index.read();

    real_t prob = probs[i];

    // Get non-zero yin_probs
    if (prob > 0) {
      // Refine peak by parabolic interpolation
      real_t period = min_period + idx + parabolic_shifts[idx];
      real_t f0 = sample_rate / period;

      // Find pitch bin corresponding to each f0 candidate
      real_t bin_idx = 12 * n_bins_per_semitone * hls::log2(f0 / FMIN);
      // Clip to range [0, n_pitch_bins-1] and round to nearest integer
      const size_t bin = hls::round(bin_idx);
      const size_t clipped_bin =
          hls::max(static_cast<size_t>(0), hls::min(n_pitch_bins - 1, bin));

      // Observation probabilities
      observation_probs[clipped_bin] += prob;
    }
  }

  // Calculate voiced probability
  real_t voiced_prob = 0.0;
  for (int i = 0; i < n_pitch_bins; ++i) {
    voiced_prob += observation_probs[i];
  }

  voiced_prob = hls::max(0.0, hls::min(1.0, voiced_prob));

  // Set unvoiced probabilities using full precision division
  real_t unvoiced_prob =
      (1.0 - voiced_prob) / static_cast<real_t>(n_pitch_bins);

  for (int i = n_pitch_bins; i < 2 * n_pitch_bins; ++i) {
    observation_probs[i] = unvoiced_prob;
  }

  return write_output(observation_probs, voiced_prob, observation_probs_stream);
}