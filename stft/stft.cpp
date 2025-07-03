#include "stft.hpp"

void stft_window(stream<real_signal, frame_length> &y_frame,
                 stream<real_t, fft_r2c_short_size> &fft_in) {
  for (index<frame_length> i = 0; i < frame_length; ++i) {
#pragma HLS PIPELINE II = 1 REWIND

    const auto fft_element = hann_window[i] * y_frame.read();

    fft_in.write(fft_element);
  }
}

void stft(stream<real_signal, frame_length> &y_frame,
          stream<complex_t, fft_r2c_short_size> &out) {
#pragma HLS DATAFLOW

  stream<real_t, fft_r2c_short_size> fft_in;

  stft_window(y_frame, fft_in);

  fft_r2c_short(fft_in, out);
}
