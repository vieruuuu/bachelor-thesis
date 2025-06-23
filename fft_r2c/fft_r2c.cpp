#include "fft_r2c.hpp"

template <typename params>
void write_forward_config_r2c(
    stream<hls::ip_fft::config_t<params>, 1> &fft_config_s) {
  static hls::ip_fft::config_t<params> config;

  config.setDir(1);

  fft_config_s.write(config);
}

template <typename params>
void write_exp_r2c(stream<hls::ip_fft::status_t<params>, 1> &fft_status_s,
                   fft_exp_stream &exp) {
  exp.write(fft_status_s.read().getBlkExp());
}

template <size_t fft_size>
void prepare_input_r2c(stream<fft_real, fft_size> &in,
                       stream<fft_complex, fft_size> &complex_in_buffer) {
  for (index<fft_size> i = 0; i < fft_size; ++i) {
#pragma HLS pipeline II = 1 rewind

    complex_in_buffer.write({in.read(), 0});
  }
}

template <size_t fft_size, typename params>
void fft_r2c_template(stream<fft_real, fft_size> &in,
                      stream<fft_complex, fft_size> &out, fft_exp_stream &exp) {
#pragma HLS dataflow

  stream<fft_complex, fft_size> complex_in_buffer;
  stream<hls::ip_fft::config_t<params>, 1> fft_config_s;
  stream<hls::ip_fft::status_t<params>, 1> fft_status_s;

  write_forward_config_r2c<params>(fft_config_s);

  prepare_input_r2c<fft_size>(in, complex_in_buffer);

  hls::fft<params>(complex_in_buffer, out, fft_status_s, fft_config_s);

  write_exp_r2c<params>(fft_status_s, exp);
}

void fft_r2c(stream<fft_real, fft_r2c_size> &in,
             stream<fft_complex, fft_r2c_size> &out, fft_exp_stream &exp) {
  return fft_r2c_template<fft_r2c_size, fft_params>(in, out, exp);
}

void fft_r2c_short(stream<fft_real, fft_r2c_short_size> &in,
                   stream<fft_complex, fft_r2c_short_size> &out,
                   fft_exp_stream &exp) {
  return fft_r2c_template<fft_r2c_short_size, fft_params_short>(in, out, exp);
}