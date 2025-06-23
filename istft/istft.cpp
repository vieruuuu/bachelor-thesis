#include "istft.hpp"

void normalize(stream<real_t, hop_length> &overlap_output,
               stream<real_t, hop_length> &window_sumsq_output,
               stream<real_t, hop_length> &output) {
  for (counter<hop_length> i = 0; i < hop_length; ++i) {
#pragma HLS PIPELINE II = 1 REWIND

    const auto overlap_element = overlap_output.read();
    const auto window_sumsq_element = window_sumsq_output.read();
    const auto tmp = window_sumsq_element > tiny
                         ? overlap_element / window_sumsq_element
                         : overlap_element;

    output.write(tmp);
  }
}

void window_sumsquare(stream<real_t, hop_length> &input,
                      stream<real_t, hop_length> &output) {

  static real_t window[frame_length] = {0.0};

  for (index<hop_length> i = 0; i < hop_length; ++i) {
#pragma HLS PIPELINE II = 1

    const auto tmp = window[i] + bh_window_sq[i].to_float();

    output.write(tmp);
  }

  for (index<frame_length> i = hop_length; i < frame_length; ++i) {
#pragma HLS PIPELINE II = 1

    const auto tmp = window[i] + bh_window_sq[i].to_float();

    window[i - hop_length] = tmp;
  }

  for (index<frame_length> i = frame_length - hop_length; i < frame_length;
       ++i) {
#pragma HLS PIPELINE II = 1

    window[i] = input.read();
  }
}

void overlap_add(stream<real_t, frame_length> &input,
                 stream<real_t, hop_length> &output) {

  static real_t window[frame_length] = {0.0};

  for (index<hop_length> i = 0; i < hop_length; ++i) {
#pragma HLS PIPELINE II = 1

    const auto tmp = window[i] + input.read();

    output.write(tmp);
  }

  for (index<frame_length> i = hop_length; i < frame_length; ++i) {
#pragma HLS PIPELINE II = 1

    const auto tmp = window[i] + input.read();

    window[i - hop_length] = tmp;
  }

  for (index<frame_length> i = frame_length - hop_length; i < frame_length;
       ++i) {
#pragma HLS PIPELINE II = 1

    window[i] = 0.0;
  }
}

void unscale_fft(stream<fft_real, frame_length> &input, fft_exp_stream &exp_in1,
                 fft_exp_stream &exp_in2,
                 stream<real_t, frame_length> &output) {
  const auto scale = exp_in1.read() + exp_in2.read();

  for (counter<frame_length> i = 0; i < frame_length; ++i) {
#pragma HLS PIPELINE II = 1 REWIND
    const auto input_scaled = static_cast<fft_real_scaled>(input.read())
                              << scale;

    output.write(input_scaled);
  }
}

void fft_irfft(stream<fft_complex, frame_length> &stft, fft_exp_stream &exp_in,
               stream<real_t, frame_length> &output) {
#pragma HLS DATAFLOW

  stream<fft_real, fft_c2r_short_size> fft_out;
  fft_exp_stream exp_out;

  fft_c2r_short(stft, fft_out, exp_out);
  unscale_fft(fft_out, exp_in, exp_out, output);
}

void istft(stream<fft_complex, frame_length> &stft, fft_exp_stream &exp_in,
           stream<real_t, hop_length> &output) {
#pragma HLS DATAFLOW

  stream<real_t, frame_length> irfft_output;
  stream<real_t, hop_length> overlap_output;
  stream<real_t, hop_length> overlap_output_array[2];
  stream<real_t, hop_length> window_sumsq_output;

  fft_irfft(stft, exp_in, irfft_output);
  overlap_add(irfft_output, overlap_output);
  duplicate_stream<real_t, hop_length, 2>(overlap_output, overlap_output_array);
  window_sumsquare(overlap_output_array[0], window_sumsq_output);
  normalize(overlap_output_array[1], window_sumsq_output, output);
}
