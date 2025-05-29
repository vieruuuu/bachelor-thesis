#include "fft_r2c.hpp"

void write_forward_config_r2c(fft_config_stream &fft_config_s) {
  static fft_config config;

  config.setDir(1);

  fft_config_s.write(config);
}

void write_exp_r2c(fft_status_stream &fft_status_s, fft_exp_stream &exp) {
  exp.write(fft_status_s.read().getBlkExp());
}

void prepare_input_r2c(fft_real_stream &in,
                       fft_complex_stream &complex_in_buffer) {
  for (index<fft_length> i = 0; i < fft_length; ++i) {
#pragma HLS pipeline II = 1 rewind

    complex_in_buffer.write({in.read(), 0});
  }
}

void fft_r2c(fft_real_stream &in, fft_complex_stream &out,
             fft_exp_stream &exp) {
#pragma HLS dataflow

  fft_complex_stream complex_in_buffer;
  fft_config_stream fft_config_s;
  fft_status_stream fft_status_s;

  write_forward_config_r2c(fft_config_s);

  prepare_input_r2c(in, complex_in_buffer);

  hls::fft<fft_params>(complex_in_buffer, out, fft_status_s, fft_config_s);

  write_exp_r2c(fft_status_s, exp);
}