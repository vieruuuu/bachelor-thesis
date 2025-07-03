#include "closest_pitch_from_scale.hpp"

Scales incScale(Scales s, int inc) {
  const auto s_num = static_cast<int>(s);
  const auto s_num_inc = s_num + inc;
  constexpr auto last_scale = ScalesCount - 1;
  constexpr auto first_scale = static_cast<int>(0);

  const auto result = hls::max(first_scale, hls::min(s_num_inc, last_scale));

  return static_cast<Scales>(result);
}

void decode_state(bool next_btn, bool prev_btn, path_stream &states_stream,
                  stream<real_t, 1> &f0_stream,
                  stream<real_t, 1> &corrected_f0_stream) {
  static bool prev_button_state = false;
  static Scales current_scale = Scales::EFLAT_MIN;

  bool current_button_state = next_btn || prev_btn;

  // Trigger only on rising edge (button was not pressed, now is)
  if (!prev_button_state && current_button_state) {
    if (next_btn) {
      current_scale = incScale(current_scale, 1);
    } else if (prev_btn) {
      current_scale = incScale(current_scale, -1);
    }
  }

  prev_button_state = current_button_state;

  const int state = states_stream.read();

  real_t f0;
  real_t corrected_f0;

  if (state < n_pitch_bins) {
    // this may be totally unnecessary
    // const int freq_idx = state % n_pitch_bins;
    const int freq_idx = state;
    const int freq_corrected_idx =
        freq_corrected_all_idx[current_scale][freq_idx];

    f0 = freqs[freq_idx];
    corrected_f0 = freq_corrected_all_unique[freq_corrected_idx];
  } else {
    f0 = std::numeric_limits<real_t>::quiet_NaN();
    corrected_f0 = std::numeric_limits<real_t>::quiet_NaN();
  }

  f0_stream.write(f0);
  corrected_f0_stream.write(corrected_f0);
}
