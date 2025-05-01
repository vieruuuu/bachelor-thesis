#include "parabolic_interpolation.hpp"

/**
 * Performs parabolic interpolation for yin algorithm
 */
void parabolic_interpolation(stream<real_t, yin_frame_size> &yin_frame,
                             stream<real_t, yin_frame_size> &shifts) {

  // First has shift of 0 (edge condition)
  shifts.write(0.0);

  real_t prev_frame;
  real_t current_frame = yin_frame.read();
  real_t next_frame = yin_frame.read();

  for (size_t i = 2; i < yin_frame_size; ++i) {
#pragma HLS PIPELINE II = 1 rewind

    prev_frame = current_frame;
    current_frame = next_frame;
    next_frame = yin_frame.read();

    // Compute coefficients for parabolic interpolation
    real_t a = next_frame + prev_frame - 2 * current_frame;
    real_t b = (next_frame - prev_frame) / 2.0;

    // Check if the estimated optimum would be outside the range [i-1, i+1]
    if (hls::abs(b) >= hls::abs(a)) {
      // If this happens, we'll shift by more than 1 bin, so set shift to 0
      shifts.write(0.0);
    } else {
      // Calculate the shift (position of the parabola optimum relative to bin
      // index)
      shifts.write(-b / a);
    }
  }

  // Last element has shift of 0 (edge condition)
  shifts.write(0.0);
}
