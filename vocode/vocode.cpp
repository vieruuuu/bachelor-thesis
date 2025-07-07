#include "vocode.hpp"

real_t mod_pos(real_t a, real_t m) {
  auto r = std::fmod(a, m); // [-m, m)

  if (r < 0) {
    r += m; // [0, m)
  }

  return r;
}

void arr(stream<complex_t, frame_length> &S, stream<real_t, 1> &original_f0_s,
         stream<real_t, 1> &corrected_f0_s,
         stream<complex_t, frame_length> &S_shifted) {

  const auto original_f0 = original_f0_s.read();
  const auto corrected_f0 = corrected_f0_s.read();
  const auto pitch_factor_nan = corrected_f0 / original_f0;
  const auto pitch_factor =
      std::isfinite(pitch_factor_nan) ? pitch_factor_nan : 1.0;

  static real_t prev_phase[nfreq] = {0.0};
  static real_t phase_acc[nfreq] = {0.0};

  complex_t dest[nfreq];

  for (index<nfreq> i = 0; i < nfreq; ++i) {
#pragma HLS PIPELINE II = 1

    const auto element = S.read();
    const auto mag =
        std::hypot(element.real(), element.imag()); // std::abs(element);
    const auto phase =
        std::atan2(element.imag(), element.real()); // std::arg(element);

    const auto delta_raw = phase - prev_phase[i] - omega[i] * hop_length;

    const auto delta =
        mod_pos(delta_raw + M_PI, static_cast<real_t>(2.0 * M_PI)) -
        static_cast<real_t>(M_PI);

    const auto true_freq = omega[i] + delta / hop_length;

    const auto phase_acc_value_raw =
        phase_acc[i] + pitch_factor * true_freq * hop_length;
    const auto phase_acc_value =
        mod_pos(phase_acc_value_raw, static_cast<real_t>(2.0 * M_PI));

    const auto S_shifted_value = std::polar(mag, phase_acc_value);

    S_shifted.write(S_shifted_value);
    dest[i] = S_shifted_value;
    phase_acc[i] = phase_acc_value;
    prev_phase[i] = phase;
  }

  for (index<nfreq> dest_bin = nfreq - 2; dest_bin > 0; --dest_bin) {
#pragma HLS PIPELINE II = 1

    S.read();

    S_shifted.write(std::conj(dest[dest_bin]));
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
