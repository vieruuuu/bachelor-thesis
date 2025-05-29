#include "fft_c2r.hpp"

void prepare_output_c2r(fft_complex_stream &complex_out_buffer,
                        fft_real_stream &out) {
  for (index<fft_length> i = 0; i < fft_length; ++i) {
#pragma HLS pipeline II = 1 rewind
    out.write(complex_out_buffer.read().real());
  }
}

void write_inverse_config_c2r(fft_config_stream &fft_config_s) {
  static fft_config config;

  config.setDir(0);

  fft_config_s.write(config);
}

void write_exp_c2r(fft_status_stream &fft_status_s, fft_exp_stream &exp) {
  exp.write(fft_status_s.read().getBlkExp());
}

void fft_c2r(fft_complex_stream &in, fft_real_stream &out,
             fft_exp_stream &exp) {
#pragma HLS dataflow

  fft_complex_stream complex_out_buffer;
  fft_config_stream fft_config_s;
  fft_status_stream fft_status_s;

  write_inverse_config_c2r(fft_config_s);

  hls::fft<fft_params>(in, complex_out_buffer, fft_status_s, fft_config_s);

  prepare_output_c2r(complex_out_buffer, out);

  write_exp_c2r(fft_status_s, exp);
}
