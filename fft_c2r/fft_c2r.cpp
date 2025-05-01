#include "fft_c2r.hpp"

void prepare_output_c2r(stream<fft_complex, frame_length> &complex_out_buffer,
                        stream<fft_real, frame_length> &out) {
  for (int i = 0; i < frame_length; ++i) {
#pragma HLS pipeline II = 1 rewind
#pragma HLS LOOP_TRIPCOUNT min = frame_length max = frame_length

    out.write(complex_out_buffer.read().real());
  }
}

void write_exp_c2r(fft_status_stream &fft_status_s,
                   stream<unsigned int, 1> &exp) {
  fft_status status = fft_status_s.read();

  exp.write(status.getBlkExp());
}

void fft_c2r(stream<fft_complex, frame_length> &in,
             stream<fft_real, frame_length> &out,
             stream<unsigned int, 1> &exp) {
#pragma HLS dataflow

  stream<fft_complex, frame_length> complex_out_buffer;

  fft_config_stream fft_config_s;
  fft_status_stream fft_status_s;

  fft_config fft_config_tmp;

  fft_config_tmp.setDir(0); // inverse fft
  // fft_config_tmp.setSch(0b011010101011);

  fft_config_s.write(fft_config_tmp);

  hls::fft<fft_params>(in, complex_out_buffer, fft_status_s, fft_config_s);

  prepare_output_c2r(complex_out_buffer, out);

  write_exp_c2r(fft_status_s, exp);
}
