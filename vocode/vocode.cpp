#include "vocode.hpp"

void arr(stream<fft_complex, frame_length> &S, fft_exp_stream &exp,
         stream<real_t, 1> &original_f0_s, stream<real_t, 1> &corrected_f0_s,
         stream<fft_complex, frame_length> &S_shifted,
         fft_exp_stream &exp_new) {
  const auto original_f0 = original_f0_s.read();
  const auto corrected_f0 = corrected_f0_s.read();

  if (std::isnan(original_f0) || std::isnan(corrected_f0)) {
    for (counter<frame_length> i = 0; i < frame_length; ++i) {
#pragma HLS PIPELINE II = 1
      S_shifted.write(S.read());
    }

    exp_new.write(exp.read());
  } else {
    const auto ratio = corrected_f0 / original_f0;
    constexpr auto nbins = frame_length / 2 + 1;
    fft_complex src[frame_length];

    for (index<frame_length> S_index = 0; S_index < frame_length; ++S_index) {
#pragma HLS PIPELINE II = 1

      src[S_index] = S.read();
    }

    stream<std::complex<real_t>, nbins> unscaled_dest;
    fft_complex scaled_dest[nbins];

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

        const auto real_element = op1 * left_src.real().to_float() +
                                  op2 * right_src.real().to_float();
        const auto imag_element = op1 * left_src.imag().to_float() +
                                  op2 * right_src.imag().to_float();

        unscaled_dest.write({real_element, imag_element});

        max_abs_value =
            hls::max(max_abs_value,
                     hls::max(hls::abs(real_element), hls::abs(imag_element)));
      } else {
        unscaled_dest.write({0, 0});
      }
    }

    const auto max_abs_value_log2 = log2_constexpr(hls::floor(max_abs_value));
    const auto max_abs_value_round =
        static_cast<real_t>(1 << max_abs_value_log2);

    for (index<nbins> dest_bin = 0; dest_bin < nbins; ++dest_bin) {
#pragma HLS PIPELINE II = 1

      const auto value = unscaled_dest.read() / max_abs_value_round;

      S_shifted.write(value);
      scaled_dest[dest_bin] = value;
    }

    for (index<nbins> dest_bin = nbins - 2; dest_bin > 0; --dest_bin) {
#pragma HLS PIPELINE II = 1

      S_shifted.write(hls::conj(scaled_dest[dest_bin]));
    }

    exp_new.write(exp.read() + max_abs_value_log2);
  }
}

// void vocode(stream<real_signal, frame_length> &y_frame,
//             stream<real_t, 1> &original_f0, stream<real_t, 1> &corrected_f0,
//             stream<fft_complex, fft_r2c_short_size> &S_shifted,
//             fft_exp_stream &exp_new
//             // stream<real_signal, hop_length> &audio_out
// ) {
// #pragma HLS DATAFLOW

//   stream<fft_complex, fft_r2c_short_size> S;
//   // stream<fft_complex, fft_r2c_short_size> S_shifted;
//   fft_exp_stream exp;
//   // fft_exp_stream exp_new;

//   stft(y_frame, S, exp);

//   arr(S, exp, original_f0, corrected_f0, S_shifted, exp_new);

// }

void vocode(stream<real_signal, frame_length> &y_frame,
            stream<real_t, 1> &original_f0, stream<real_t, 1> &corrected_f0,
            stream<real_t, hop_length> &audio_out) {
#pragma HLS DATAFLOW

  stream<fft_complex, fft_r2c_short_size> S;
  stream<fft_complex, fft_r2c_short_size> S_shifted;
  fft_exp_stream exp;
  fft_exp_stream exp_new;

  stft(y_frame, S, exp);

  arr(S, exp, original_f0, corrected_f0, S_shifted, exp_new);

  istft(S_shifted, exp_new, audio_out);
}
