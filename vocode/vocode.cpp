#include "vocode.hpp"

void arr(stream<complex_t, frame_length> &S, stream<real_t, 1> &original_f0_s,
         stream<real_t, 1> &corrected_f0_s,
         stream<complex_t, frame_length> &S_shifted) {
  const auto original_f0 = original_f0_s.read();
  const auto corrected_f0 = corrected_f0_s.read();

  if (std::isnan(original_f0) || std::isnan(corrected_f0)) {
    for (counter<frame_length> i = 0; i < frame_length; ++i) {
#pragma HLS PIPELINE II = 1
      S_shifted.write(S.read());
    }
  } else {
    const auto ratio = corrected_f0 / original_f0;
    constexpr auto nbins = frame_length / 2 + 1;
    complex_t src[frame_length];

    for (index<frame_length> S_index = 0; S_index < frame_length; ++S_index) {
#pragma HLS PIPELINE II = 1

      src[S_index] = S.read();
    }

    complex_t scaled_dest[nbins];

    real_t max_abs_value = -std::numeric_limits<real_t>::infinity();

    for (index<nbins> dest_bin = 0; dest_bin < nbins; ++dest_bin) {
#pragma HLS PIPELINE II = 1

      const auto src_pos = dest_bin / ratio;
      const auto valid = (src_pos >= 0) && (src_pos < nbins);

      if (valid) {
        const auto left = static_cast<index<nbins>>(hls::floor(src_pos));
        const auto right = left + 1;

        const auto left_clipped =
            hls::max(size_t(0), hls::min(left, nbins - 1));
        const auto right_clipped =
            hls::max(size_t(0), hls::min(right, nbins - 1));

        const auto frac = src_pos - left;
        const auto op1 = 1 - frac;
        const auto op2 = frac;

        const auto left_src = src[left_clipped];
        const auto right_src = src[right_clipped];

        const auto real_element =
            op1 * left_src.real() + op2 * right_src.real();
        const auto imag_element =
            op1 * left_src.imag() + op2 * right_src.imag();

        const auto tmp = complex_t(real_element, imag_element);

        scaled_dest[dest_bin] = tmp;
        S_shifted.write(tmp);
      } else {
        scaled_dest[dest_bin] = {0, 0};
        S_shifted.write({0, 0});
      }
    }

    for (index<nbins> dest_bin = nbins - 2; dest_bin > 0; --dest_bin) {
#pragma HLS PIPELINE II = 1

      S_shifted.write(hls::conj(scaled_dest[dest_bin]));
    }
  }
}

void vocode(stream<real_signal, frame_length> &y_frame,
            stream<real_t, 1> &original_f0, stream<real_t, 1> &corrected_f0,
            stream<real_t, hop_length> &audio_out) {
#pragma HLS DATAFLOW

  stream<complex_t, fft_r2c_short_size> S;
  stream<complex_t, fft_r2c_short_size> S_shifted;

  stft(y_frame, S);

  arr(S, original_f0, corrected_f0, S_shifted);

  istft(S_shifted, audio_out);
}
