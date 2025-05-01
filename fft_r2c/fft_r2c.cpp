#include "fft_r2c.hpp"

void prepare_input_r2c(stream<fft_real, frame_length> &in,
                       stream<fft_complex, frame_length> &complex_in_buffer) {
  fft_complex tmp = {0, 0};

  for (int i = 0; i < frame_length; ++i) {
#pragma HLS pipeline II = 1 rewind
#pragma HLS LOOP_TRIPCOUNT min = frame_length max = frame_length

    tmp.real(in.read());

    complex_in_buffer.write(tmp);
  }
}

void write_exp_r2c(fft_status_stream &fft_status_s,
                   stream<unsigned int, 1> &exp) {
  fft_status status = fft_status_s.read();

  exp.write(status.getBlkExp());
}

void fft_r2c(stream<fft_real, frame_length> &in,
             stream<fft_complex, frame_length> &out,
             stream<unsigned int, 1> &exp) {
#pragma HLS dataflow

  stream<fft_complex, frame_length> complex_in_buffer;

  fft_config_stream fft_config_s;
  fft_status_stream fft_status_s;

  fft_config fft_config_tmp;

  fft_config_tmp.setDir(1); // forward fft
  // fft_config_tmp.setSch(0b011010101011);

  fft_config_s.write(fft_config_tmp);

  prepare_input_r2c(in, complex_in_buffer);

  hls::fft<fft_params>(complex_in_buffer, out, fft_status_s, fft_config_s);

  write_exp_r2c(fft_status_s, exp);
}