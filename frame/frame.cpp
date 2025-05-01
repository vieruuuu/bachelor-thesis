#include "frame.hpp"

/**
 * Frame an audio signal into overlapping frames
 */
void frame(stream<real_t, hop_length> &y,
           stream<real_t, frame_length> &y_frame) {
  static real_t buffer[frame_length] = {0};

  // shift_left_buffer
  for (int i = 0; i < frame_length - hop_length; ++i) {
#pragma HLS PIPELINE II = 1 rewind
    real_t tmp = buffer[i + hop_length];

    y_frame.write(tmp);
    buffer[i] = tmp;
  }

  // read_elements
  for (int i = frame_length - hop_length; i < frame_length; ++i) {
#pragma HLS PIPELINE II = 1 rewind
    real_t tmp = y.read();

    y_frame.write(tmp);
    buffer[i] = tmp;
  }
}
