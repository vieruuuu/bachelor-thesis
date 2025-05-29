#include "r2c_fft.hpp"

void r2c_prepare_input(r2c_real_stream &in,
                       r2c_complex_stream_buffer complex_in_buffer) {

  for (index<r2c_sample> t = 0; t < r2c_sample; ++t) {
#pragma HLS PIPELINE II = 1 rewind

    for (index<r2c_r> r = 0; r < r2c_r; ++r) {
#pragma HLS UNROLL
      complex_in_buffer[r].write({in.read(), 0.0});
    }
  }
}

void r2c_prepare_output(r2c_complex_stream_buffer complex_out_buffer,
                        r2c_complex_stream &out) {
  for (index<r2c_sample> t = 0; t < r2c_sample; ++t) {
#pragma HLS PIPELINE II = 1 rewind

    for (index<r2c_r> r = 0; r < r2c_r; ++r) {
#pragma HLS UNROLL

      out.write(complex_out_buffer[r].read());
    }
  }
}

constexpr size_t r2c_fft_id_1 = 1;
void r2c_fft_1(r2c_real_stream &in, r2c_complex_stream &out) {
#pragma HLS dataflow

  r2c_complex_stream_buffer complex_in_buffer;
  r2c_complex_stream_buffer complex_out_buffer;

  r2c_prepare_input(in, complex_in_buffer);

  xf::dsp::fft::fft<r2c_params, r2c_fft_id_1, r2c_complex>(complex_in_buffer,
                                                           complex_out_buffer);

  r2c_prepare_output(complex_out_buffer, out);
}

constexpr size_t r2c_fft_id_2 = 2;
void r2c_fft_2(r2c_real_stream &in, r2c_complex_stream &out) {
#pragma HLS dataflow

  r2c_complex_stream_buffer complex_in_buffer;
  r2c_complex_stream_buffer complex_out_buffer;

  r2c_prepare_input(in, complex_in_buffer);

  xf::dsp::fft::fft<r2c_params, r2c_fft_id_2, r2c_complex>(complex_in_buffer,
                                                           complex_out_buffer);

  r2c_prepare_output(complex_out_buffer, out);
}

void r2c_fft_top(r2c_real_stream &in, r2c_complex_stream &out) {
  return r2c_fft_1(in, out);
}