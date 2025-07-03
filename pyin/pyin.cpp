#include "pyin.hpp"

void pyin(stream<real_signal, frame_length> &y_frame,
          stream<real_t, 1> &f0_stream, stream<real_t, 1> &corrected_f0_stream,
          bool next_btn, bool prev_btn) {
#pragma HLS DATAFLOW

  stream<real_t, yin_frame_size> yin_frame;
  stream<real_t, yin_frame_size> yin_frame_array[2];
  stream<real_t, yin_frame_size> parabolic_shifts;
  stream<real_t, 2 * n_pitch_bins> observation_probs_stream;
  path_stream states_stream;

  // Compute yin for each frame.
  cumulative_mean_normalized_difference(y_frame, yin_frame);

  duplicate_stream<real_t, yin_frame_size, 2>(yin_frame, yin_frame_array);

  parabolic_interpolation(yin_frame_array[0], parabolic_shifts);

  pyin_helper(yin_frame_array[1], parabolic_shifts, observation_probs_stream);

  online_windowed_viterbi(observation_probs_stream, states_stream);

  decode_state(next_btn, prev_btn, states_stream, f0_stream,
               corrected_f0_stream);
}