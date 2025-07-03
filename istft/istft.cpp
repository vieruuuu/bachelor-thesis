#include "istft.hpp"

void normalize(stream<real_t, hop_length> &overlap_output,
               //  stream<real_t, hop_length> &window_sumsq_output,
               stream<real_t, hop_length> &output) {
  for (counter<hop_length> i = 0; i < hop_length; ++i) {
#pragma HLS PIPELINE II = 1 REWIND

    const auto overlap_element = overlap_output.read();
    // const auto window_sumsq_element = window_sumsq_output.read();

    // const auto tmp = window_sumsq_element > 1e-8
    //                      ? overlap_element / window_sumsq_element
    //                      : 0.0;

    output.write(overlap_element);
  }
}

// void window_sumsquare(stream<real_t, hop_length> &input,
//                       stream<real_t, hop_length> &output) {

//   static real_t window[frame_length] = {0.0};

//   for (index<hop_length> i = 0; i < hop_length; ++i) {
// #pragma HLS PIPELINE II = 1

//     const auto tmp = window[i] + bh_window_sq[i].to_float();

//     output.write(tmp);
//   }

//   for (index<frame_length> i = hop_length; i < frame_length; ++i) {
// #pragma HLS PIPELINE II = 1

//     const auto tmp = window[i] + bh_window_sq[i].to_float();

//     window[i - hop_length] = tmp;
//   }

//   for (index<frame_length> i = frame_length - hop_length; i < frame_length;
//        ++i) {
// #pragma HLS PIPELINE II = 1

//     window[i] = input.read();
//   }
// }

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

constexpr auto inv_a = 1.0 / fft_c2r_short_size;

void unscale_fft(stream<real_t, frame_length> &input,
                 stream<real_t, frame_length> &output) {

  for (counter<frame_length> i = 0; i < frame_length; ++i) {
#pragma HLS PIPELINE II = 1 REWIND

    output.write(input.read() * inv_a);
  }
}

void fft_irfft(stream<complex_t, frame_length> &stft,
               stream<real_t, frame_length> &output) {
#pragma HLS DATAFLOW

  stream<real_t, fft_c2r_short_size> fft_out;

  fft_c2r_short(stft, fft_out);
  unscale_fft(fft_out, output);
}

void istft(stream<complex_t, frame_length> &stft,
           stream<real_t, hop_length> &output) {
#pragma HLS DATAFLOW

  stream<real_t, frame_length> irfft_output;
  stream<real_t, hop_length> overlap_output;
  stream<real_t, hop_length> overlap_output_array[2];
  stream<real_t, hop_length> window_sumsq_output;

  fft_irfft(stft, irfft_output);
  overlap_add(irfft_output, overlap_output);
  // duplicate_stream<real_t, hop_length, 2>(overlap_output,
  // overlap_output_array); window_sumsquare(overlap_output_array[0],
  // window_sumsq_output);
  normalize(overlap_output,
            // overlap_output_array[1],
            // window_sumsq_output,
            output);
}
