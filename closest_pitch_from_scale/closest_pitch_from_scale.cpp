#include "closest_pitch_from_scale.hpp"

void decode_state(Scales scale, path_stream &states_stream,
                  stream<real_t, 1> &f0_stream,
                  stream<real_t, 1> &corrected_f0_stream) {
  const int state = states_stream.read();

  real_t f0;
  real_t corrected_f0;

  if (state < n_pitch_bins) {
    // this may be totally unnecessary
    // const int freq_idx = state % n_pitch_bins;
    const int freq_idx = state;
    const int freq_corrected_idx = freq_corrected_all_idx[scale][freq_idx];

    f0 = freqs[freq_idx];
    corrected_f0 = freq_corrected_all_unique[freq_corrected_idx];
  } else {
    f0 = std::numeric_limits<real_t>::quiet_NaN();
    corrected_f0 = std::numeric_limits<real_t>::quiet_NaN();
  }

  f0_stream.write(f0);
  corrected_f0_stream.write(corrected_f0);
}
