#include "frame.hpp"

using FrameWindow = SlidingWindow<real_signal, hop_length, frame_length>;

/**
 * Frame an audio signal into overlapping frames
 */
void frame(stream<real_signal, hop_length> &y,
           stream<real_signal, frame_length> &y_frame) {
#pragma HLS DATAFLOW

  static FrameWindow window(0.0);

  window.slideLeftStreaming(y, y_frame);
}
