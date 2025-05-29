#include "c2r_fft.hpp"

void c2r_prepare_input(c2r_complex_stream &in,
                       c2r_complex_stream_buffer complex_in_buffer) {
  for (index<c2r_sample> t = 0; t < c2r_sample; ++t) {
#pragma HLS PIPELINE II = 1 rewind

    for (index<c2r_r> r = 0; r < c2r_r; ++r) {
#pragma HLS UNROLL

      complex_in_buffer[r].write(in.read());
    }
  }
}

void c2r_prepare_output(c2r_complex_stream_buffer complex_out_buffer,
                        c2r_real_stream &out) {
  for (index<c2r_sample> t = 0; t < c2r_sample; ++t) {
#pragma HLS PIPELINE II = 1 rewind

    for (index<c2r_r> r = 0; r < c2r_r; ++r) {
#pragma HLS UNROLL

      out.write(complex_out_buffer[r].read().real());
    }
  }
}

constexpr size_t c2r_fft_id_1 = 3;
void c2r_fft_1(c2r_complex_stream &in, c2r_real_stream &out) {
#pragma HLS dataflow

  c2r_complex_stream_buffer complex_in_buffer;
  c2r_complex_stream_buffer complex_out_buffer;

  c2r_prepare_input(in, complex_in_buffer);

  xf::dsp::fft::fft<c2r_params, c2r_fft_id_1, c2r_complex>(complex_in_buffer,
                                                           complex_out_buffer);

  c2r_prepare_output(complex_out_buffer, out);
}

void c2r_fft_top(c2r_complex_stream &in, c2r_real_stream &out) {
  return c2r_fft_1(in, out);
}