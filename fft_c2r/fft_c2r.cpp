#include "fft_c2r.hpp"

template <size_t fft_size>
void prepare_output_c2r(stream<complex_t, fft_size> &complex_out_buffer,
                        stream<real_t, fft_size> &out) {
  for (counter<fft_size> i = 0; i < fft_size; ++i) {
#pragma HLS pipeline II = 1 rewind
    out.write(complex_out_buffer.read().real());
  }
}

template <typename params>
void write_inverse_config_c2r(
    stream<hls::ip_fft::config_t<params>, 1> &fft_config_s) {
  static hls::ip_fft::config_t<params> config;

  config.setDir(0);

  fft_config_s.write(config);
}

template <size_t fft_size, typename params>
void fft_c2r_template(stream<complex_t, fft_size> &in,
                      stream<real_t, fft_size> &out) {
#pragma HLS dataflow

  stream<complex_t, fft_size> complex_out_buffer;
  stream<hls::ip_fft::config_t<params>, 1> fft_config_s;
  stream<hls::ip_fft::status_t<params>, 1> fft_status_s;

  write_inverse_config_c2r<params>(fft_config_s);

  hls::fft<params>(in, complex_out_buffer, fft_status_s, fft_config_s);

  prepare_output_c2r<fft_size>(complex_out_buffer, out);

  // discard status
  fft_status_s.read();
}

void fft_c2r(stream<complex_t, fft_c2r_size> &in,
             stream<real_t, fft_c2r_size> &out) {
  return fft_c2r_template<fft_c2r_size, fft_params>(in, out);
}

void fft_c2r_short(stream<complex_t, fft_c2r_short_size> &in,
                   stream<real_t, fft_c2r_short_size> &out) {
  return fft_c2r_template<fft_c2r_short_size, fft_params_short>(in, out);
}
