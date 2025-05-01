#include "pyin.hpp"
#include <fstream>
#include <iostream>
#include <tuple>

void decode_state(path_stream &state_stream, stream<real_t, 1> &f0) {
  int state = state_stream.read();

  if (state < n_pitch_bins) {
    f0.write(freqs[state % n_pitch_bins]);
  } else {
    f0.write(NAN);
  }
}

// discard voiced_prob as its not used anywhere at the moment
// and might be deleted
void discard_voiced_prob(stream<real_t, 1> &voiced_prob_stream) {
  voiced_prob_stream.read();
}

void pyin(stream<real_t, hop_length> &y, stream<real_t, 1> &f0) {
#pragma HLS DATAFLOW

  stream<real_t, frame_length> y_frame;
  stream<real_t, yin_frame_size> yin_frame1;
  stream<real_t, yin_frame_size> yin_frame2;
  stream<real_t, yin_frame_size> parabolic_shifts;
  stream<real_t, 2 * n_pitch_bins> observation_probs_stream;
  stream<real_t, 1> voiced_prob_stream;
  path_stream state_stream;

  frame(y, y_frame);

  // Compute yin for each frame.
  cumulative_mean_normalized_difference(y_frame, yin_frame1, yin_frame2);

  // Parabolic interpolation.
  parabolic_interpolation(yin_frame1, parabolic_shifts);

  pyin_helper(yin_frame2, parabolic_shifts, observation_probs_stream,
              voiced_prob_stream);

  discard_voiced_prob(voiced_prob_stream);

  greedy_decode_lookahead_stream(observation_probs_stream, state_stream);

  decode_state(state_stream, f0);
}