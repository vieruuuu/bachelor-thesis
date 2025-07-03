#include "istft.hpp"

constexpr real_t kHannGain = 4.0 / 3.0; // 1 / 0.75

/**********************************************************************
 *  Overlap–add with built-in windowing (real-time, no divisions)
 *********************************************************************/
void overlap_add_window(stream<real_t, frame_length> &input,
                        stream<real_t, hop_length> &output) {

  static real_t carry[frame_length] = {0.0}; // circular overlap buffer

  /* ----------------------------------------------------------------
   * (1) Produce the next hop worth of PCM samples
   * ---------------------------------------------------------------- */
  for (index<hop_length> i = 0; i < hop_length; ++i) {
#pragma HLS PIPELINE II = 1

    const real_t w =
        hann_window[i].to_float() * kHannGain; // or hann_window_scaled[i]
    const real_t x = input.read() * w;
    const real_t y = carry[i] + x; // OLA

    output.write(y);
  }

  /* ----------------------------------------------------------------
   * (2) Shift the tail of the buffer left by hop and add new data
   * ---------------------------------------------------------------- */
  for (index<frame_length> i = hop_length; i < frame_length; ++i) {
#pragma HLS PIPELINE II = 1

    const real_t w = hann_window[i].to_float() * kHannGain;
    const real_t x = input.read() * w;

    carry[i - hop_length] = carry[i] + x; // prepare next call
  }

  /* ----------------------------------------------------------------
   * (3) Zero the now-unused upper hop section of the carry buffer
   * ---------------------------------------------------------------- */
  for (index<frame_length> i = frame_length - hop_length; i < frame_length;
       ++i) {
#pragma HLS PIPELINE II = 1
    carry[i] = 0.0;
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

/**********************************************************************
 *  Inverse STFT top-level (simplified DATAFLOW graph)
 *********************************************************************/
void istft(stream<complex_t, frame_length> &stft,
           stream<real_t, hop_length> &output) {
#pragma HLS DATAFLOW

  stream<real_t, frame_length> irfft_output;

  fft_irfft(stft, irfft_output); // ← unchanged
  overlap_add_window(irfft_output, output);
}
