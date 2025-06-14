#include "pyin_helper.hpp"

static const auto fmin_log2 = hls::log2(FMIN);
static const auto sample_rate_log2 = hls::log2(sample_rate);

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
    const bool is_trough_value =
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

void final(real_t unvoiced_prob,
           const real_t observation_probs_half[n_pitch_bins],
           stream<real_t, 2 * n_pitch_bins> &observation_probs) {

  for (index<n_pitch_bins> i = 0; i < n_pitch_bins; ++i) {
#pragma HLS PIPELINE II = 1
    observation_probs.write(observation_probs_half[i]);
  }

  for (index<2 * n_pitch_bins> i = n_pitch_bins; i < 2 * n_pitch_bins; ++i) {
#pragma HLS PIPELINE II = 1
    observation_probs.write(unvoiced_prob);
  }
}

void pyin_helper(stream<real_t, yin_frame_size> &yin_frame_stream,
                 stream<real_t, yin_frame_size> &parabolic_shifts_stream,
                 stream<real_t, 2 * n_pitch_bins> &observation_probs) {

  stream<real_t, yin_frame_size> trough_heights;
  stream<bool, yin_frame_size> is_trough;
  stream<int, yin_frame_size> trough_index;

  bool trough_thresholds[yin_frame_size][thresholds_size - 1] = {false};
  real_t probs[yin_frame_size] = {0.0};
  real_t yin_frame[yin_frame_size];
  real_t parabolic_shifts[yin_frame_size];
  real_t observation_probs_half[n_pitch_bins] = {0.0};

  auto global_min_value = std::numeric_limits<real_t>::infinity();
  index<yin_frame_size> global_min_index = 0;

  copy_stream_to_buffer(yin_frame_stream, parabolic_shifts_stream, yin_frame,
                        parabolic_shifts);

  // Find the troughs (local minima)
  localmin(yin_frame, is_trough);

  index<yin_frame_size> last_trough_index = 0;
  {
    for (index<yin_frame_size> i = 0; i < yin_frame_size; ++i) {
      const auto is_trough_value = is_trough.read();

      if (!is_trough_value) {
        continue;
      }

      trough_index.write(i);

      // Get trough heights
      const auto trough_height = yin_frame[i];
      trough_heights.write(trough_height);

      // Find global minimum
      if (trough_height < global_min_value) {
        global_min_value = trough_height;
        global_min_index = last_trough_index;
      }

      last_trough_index += 1;
    }
  }

  const index<yin_frame_size> trough_thresholds_size = last_trough_index;

  index<thresholds_size> n_thresholds_below_min = 0;
  int col_sum[thresholds_size - 1] = {0};
  {
    // Find which troughs are below each threshold
    for (index<yin_frame_size> i = 0; i < trough_thresholds_size; ++i) {
#pragma HLS LOOP_TRIPCOUNT max = yin_frame_size

      const auto trough_height = trough_heights.read();
      const auto is_global_min_index = i == global_min_index;

      for (index<thresholds_size - 1> j = 0; j < thresholds_size - 1; ++j) {
        const auto trough_threshold_value = trough_height < thresholds[j + 1];

        n_thresholds_below_min +=
            is_global_min_index && !trough_threshold_value;
        trough_thresholds[i][j] = trough_threshold_value;
        col_sum[j] += trough_threshold_value;
      }
    }
  }
  // Sum beta probabilities for thresholds below minimum
  const auto beta_sum = beta_sums[n_thresholds_below_min];

  probs[global_min_index] += no_trough_prob * beta_sum;

  // Define the prior over the troughs using Boltzmann distribution
  for (size_t j = 0; j < thresholds_size - 1; ++j) {
    const auto trough_count = col_sum[j];

    if (trough_count > 0) {
      const auto beta_prob = beta_probs[j];

      int cum_sum = 0;

    boltzmann:
      for (size_t i = 0; i < trough_thresholds_size; ++i) {
#pragma HLS LOOP_TRIPCOUNT max = yin_frame_size

        // Calculate trough prior using Boltzmann distribution
        if (trough_thresholds[i][j]) {
          const auto trough_prior = boltzmann_pmf(cum_sum, trough_count);

          // For each threshold add probability to global minimum if no trough
          // is below threshold, else add probability to each trough below
          // threshold biased by prior
          probs[i] += trough_prior * beta_prob;

          cum_sum++;
        }
      }
    }
  }

  // Calculate voiced probability
  auto voiced_prob = static_cast<real_t>(0.0);

LOG2_LOOP:
  for (index<yin_frame_size> i = 0; i < trough_thresholds_size; ++i) {
#pragma HLS LOOP_TRIPCOUNT max = yin_frame_size

    const auto idx = trough_index.read();
    const auto prob = probs[i];

    // Get non-zero yin_probs
    if (prob > 0) {
      // Refine peak by parabolic interpolation
      const auto period = min_period + idx + parabolic_shifts[idx];
      const auto period_log2 = hls::log2(period);
      const auto f0_log2 = sample_rate_log2 - period_log2;

      // Find pitch bin corresponding to each f0 candidate
      const auto bin_idx = 12 * n_bins_per_semitone * (f0_log2 - fmin_log2);
      // Clip to range [0, n_pitch_bins-1] and round to nearest integer
      const auto bin = static_cast<size_t>(hls::round(bin_idx));
      const auto clipped_bin =
          hls::max(static_cast<size_t>(0), hls::min(n_pitch_bins - 1, bin));

      // Observation probabilities
      observation_probs_half[clipped_bin] += prob;
      voiced_prob += prob;
    }
  }

  voiced_prob = hls::max(0.0, hls::min(1.0, voiced_prob));

  // Set unvoiced probabilities using full precision division
  const auto unvoiced_prob =
      (1.0 - voiced_prob) / static_cast<real_t>(n_pitch_bins);

  final(unvoiced_prob, observation_probs_half, observation_probs);
}
