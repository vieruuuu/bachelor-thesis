#include "stft.hpp"

void stft_window(stream<real_signal, frame_length> &y_frame,
                 stream<fft_real, fft_r2c_short_size> &fft_in) {
  for (index<frame_length> i = 0; i < frame_length; ++i) {
#pragma HLS PIPELINE II = 1

    const auto fft_element = bh_window[i] * y_frame.read();

    fft_in.write(fft_element);
  }
}

void stft(stream<real_signal, frame_length> &y_frame,
          stream<fft_complex, fft_r2c_short_size> &out, fft_exp_stream &exp) {
#pragma HLS DATAFLOW

  stream<fft_real, fft_r2c_short_size> fft_in;

  stft_window(y_frame, fft_in);

  fft_r2c_short(fft_in, out, exp);
}
