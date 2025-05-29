#include "cmnd.hpp"
#include <iostream>

constexpr size_t K = max_period;
constexpr size_t n = frame_length;
constexpr size_t max_size = max_period + 1;
constexpr size_t out_size = max_size;
constexpr size_t n_pad = 2 * n;
constexpr size_t n_bins = n_pad / 2 + 1;
void autocorrelate_new(stream<real_t, frame_length> &y,
                       stream<real_t, out_size> &acf,
                       stream<real_t, K> &cumsum_sq) {
  // Determine output size
  // Zero-pad length for circular convolution: at least 2*n for full linear
  // autocorrelation
  // Allocate input and output buffers
  r2c_real_stream r2c_in;
  r2c_complex_stream r2c_out;
  c2r_complex_stream c2r_in;
  c2r_real_stream c2r_out;
  real_t previous_cumsum_sq = 0.0;

  for (size_t i = 0; i < frame_length; ++i) {
    const real_t current_y_frame = y.read();

    r2c_in.write(current_y_frame);

    if (i < K) {
      const real_t tmp = current_y_frame * current_y_frame + previous_cumsum_sq;

      previous_cumsum_sq = tmp;

      cumsum_sq.write(tmp);
    }
  }

  for (size_t i = frame_length; i < n_pad; ++i) {
    r2c_in.write(0.0);
  }

  // Plan and execute forward transform
  r2c_fft_1(r2c_in, r2c_out);

  // Compute power spectrum (abs^2)
  for (size_t i = 0; i < n_pad; ++i) {
    const r2c_complex tmp = r2c_out.read();

    if (i < n_bins) {
      const real_t re = tmp.real();
      const real_t im = tmp.imag();
      const real_t mag2 = re * re + im * im;

      c2r_in.write({mag2, 0.0});
    } else {
      c2r_in.write({0.0, 0.0});
    }
  }

  // Plan and execute inverse transform
  c2r_fft_1(c2r_in, c2r_out);

  // Normalize inverse FFT
  for (size_t i = 0; i < n_pad; ++i) {
    const c2r_real tmp = c2r_out.read();

    // Extract autocorrelation for lags [0, out_size)
    if (i < out_size) {
      const real_t acf_value = tmp * 2;
      // std::cout << "acf [" << i << "]: " << acf_value << std::endl;
      acf.write(acf_value);
    }
  }
}

// 3) Difference function d[k][f] for k=1..K
//    d(0)=0 by definition
// for k >= 1: d[k] = 2*(acf[0] - acf[k]) - sum_{m=0}^{k-1} y^2[m]
// 4) Cumulative mean normalized difference:
//    yin[k][f] = d[k][f] / ( (1/k) * sum_{j=1}^k d[j][f] )
// we only store k in [min_period..max_period]
// Precompute prefix sums of d for k=1..K
// Fill output
void diff_cmnd_output(stream<real_t, K> &cumsum_sq, stream<real_t, K + 1> &acf,
                      stream<real_t, yin_frame_size> &yin_frame1,
                      stream<real_t, yin_frame_size> &yin_frame2) {
  real_t d_cumsum_running = 0.0;
  const real_t first_acf = acf.read();

  for (size_t k = 1; k <= K; ++k) {
#pragma HLS PIPELINE II = 1 rewind

    const real_t current_acf = acf.read();
    const real_t sum_sq = cumsum_sq.read();

    const real_t dk = 2.0 * (first_acf - current_acf) - sum_sq;

    d_cumsum_running += dk;

    if (k >= min_period && k <= max_period) {
      const real_t cm = d_cumsum_running / static_cast<real_t>(k);
      const real_t result = dk / (cm + tiny);

      yin_frame1.write(result);
      yin_frame2.write(result);
    }
  }
}

// y_frames: [frame_length][n_frames]
// min_period, max_period: >0, with max_period < frame_length
// returns yin: [(max_period-min_period+1)][n_frames]
void cumulative_mean_normalized_difference(
    stream<real_t, frame_length> &y_frame_stream,
    stream<real_t, yin_frame_size> &yin_frame1,
    stream<real_t, yin_frame_size> &yin_frame2) {
#pragma HLS DATAFLOW

  stream<real_t, K + 1> acf;
  stream<real_t, K> cumsum_sq;

  autocorrelate_new(y_frame_stream, acf, cumsum_sq);

  diff_cmnd_output(cumsum_sq, acf, yin_frame1, yin_frame2);
}
