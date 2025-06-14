#include "pyin.hpp"

void frame(stream<real_signal, hop_length> &y,
           stream<real_signal, frame_length> &y_frame) {
  static real_signal buffer[frame_length] = {0};

  // shift_left_buffer
  for (int i = 0; i < frame_length - hop_length; ++i) {
#pragma HLS PIPELINE II = 1 rewind
    const real_signal tmp = buffer[i + hop_length];

    y_frame.write(tmp);
    buffer[i] = tmp;
  }

  // read_elements
  for (int i = frame_length - hop_length; i < frame_length; ++i) {
#pragma HLS PIPELINE II = 1 rewind
    const real_signal tmp = y.read();

    y_frame.write(tmp);
    buffer[i] = tmp;
  }
}

void pyin(stream<real_signal, hop_length> &y, stream<real_t, 1> &f0_stream,
          stream<real_t, 1> &corrected_f0_stream, Scales scale) {
#pragma HLS DATAFLOW

  stream<real_signal, frame_length> y_frame;
  stream<real_t, yin_frame_size> yin_frame1;
  stream<real_t, yin_frame_size> yin_frame2;
  stream<real_t, yin_frame_size> parabolic_shifts;
  stream<real_t, 2 * n_pitch_bins> observation_probs_stream;
  path_stream states_stream;

  frame(y, y_frame);

  // Compute yin for each frame.
  cumulative_mean_normalized_difference(y_frame, yin_frame1, yin_frame2);

  // Parabolic interpolation.
  parabolic_interpolation(yin_frame1, parabolic_shifts);

  pyin_helper(yin_frame2, parabolic_shifts, observation_probs_stream);

  // greedy_decode_lookahead_stream(observation_probs_stream, states_stream);
  online_windowed_viterbi(observation_probs_stream, states_stream);

  decode_state(scale, states_stream, f0_stream, corrected_f0_stream);
}