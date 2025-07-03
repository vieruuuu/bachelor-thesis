#include "cmnd.hpp"
#include <iostream>

constexpr size_t K = max_period;
constexpr size_t n = frame_length;
constexpr size_t max_size = max_period + 1;
constexpr size_t out_size = max_size;
constexpr size_t n_pad = 2 * n;
constexpr size_t n_bins = n_pad / 2 + 1;

// p=a*a+b*b where a=[-1, 1] b=[-1, 1] -> p=[-2, 2]
// p is scaled down by 1
constexpr int PRODUCT_SCALE_FACTOR = 1;
// nu stiu de ce e nevoie de el ngl dar merge asa
constexpr int UNKOWN_SCALE_FACTOR = 1;
// acf is doubled when calculating dk
constexpr int ACF_SCALE_FACTOR = 1;

constexpr int STATIC_SCALE_FACTOR = fft_params::max_nfft -
                                    PRODUCT_SCALE_FACTOR - ACF_SCALE_FACTOR -
                                    UNKOWN_SCALE_FACTOR;

void asd(stream<real_signal, frame_length> &y,
         stream<real_t, fft_r2c_size> &r2c_in, stream<real_t, K> &cumsum_sq) {
  auto previous_cumsum_sq = real_t(0.0);

COMPUTE_CUMSUM_SQ:
  for (counter<K> i = 0; i < K; ++i) {
#pragma HLS PIPELINE II = 1

    const auto current_y_frame = y.read();

    r2c_in.write(current_y_frame);

    const auto current_y_frame_sq = current_y_frame * current_y_frame;

    previous_cumsum_sq += current_y_frame_sq.to_float();

    cumsum_sq.write(previous_cumsum_sq);
  }

READ_REMAINING_Y:
  for (counter<frame_length - K> i = 0; i < frame_length - K; ++i) {
#pragma HLS PIPELINE II = 1

    r2c_in.write(y.read());
  }

ADD_PADDING:
  for (counter<n_pad - frame_length> i = 0; i < n_pad - frame_length; ++i) {
#pragma HLS PIPELINE II = 1

    r2c_in.write(0.0);
  }
}

void compute_power_spectrum(stream<complex_t, fft_r2c_size> &r2c_out,
                            stream<complex_t, fft_r2c_size> &c2r_in) {

COMPUTE_MAG2:
  for (counter<n_bins> i = 0; i < n_bins; ++i) {
#pragma HLS PIPELINE II = 1
    const auto tmp = r2c_out.read();
    const auto mag2 = tmp.real() * tmp.real() + tmp.imag() * tmp.imag();

    c2r_in.write({mag2, 0.0});
  }

ADD_PADDING:
  for (counter<n_pad - n_bins> i = 0; i < n_pad - n_bins; ++i) {
#pragma HLS PIPELINE II = 1

    c2r_in.write({0.0, 0.0});

    // discard fft output
    r2c_out.read();
  }
}

void normalization(stream<real_t, fft_r2c_size> &c2r_out,
                   stream<real_t, out_size> &acf) {
  constexpr real_t scale = 1.0 / fft_c2r_size;

EXTRACT_AUTOCORRELATION:
  for (counter<out_size> i = 0; i < out_size; ++i) {
#pragma HLS PIPELINE II = 1

    acf.write(c2r_out.read() * scale);
  }

DISCARD_FFT_OUTPUT:
  for (counter<n_pad - out_size> i = 0; i < n_pad - out_size; ++i) {
#pragma HLS PIPELINE II = 1
    c2r_out.read();
  }
}

void autocorrelate_new(stream<real_signal, frame_length> &y,
                       stream<real_t, out_size> &acf,
                       stream<real_t, K> &cumsum_sq) {
#pragma HLS DATAFLOW

  // Determine output size
  // Zero-pad length for circular convolution: at least 2*n for full linear
  // autocorrelation
  // Allocate input and output buffers
  stream<real_t, fft_r2c_size> r2c_in;
  stream<complex_t, fft_r2c_size> r2c_out;
  stream<complex_t, fft_r2c_size> c2r_in;
  stream<real_t, fft_r2c_size> c2r_out;

  asd(y, r2c_in, cumsum_sq);

  // Plan and execute forward transform
  fft_r2c(r2c_in, r2c_out);

  // Compute power spectrum (abs^2)
  compute_power_spectrum(r2c_out, c2r_in);

  // Plan and execute inverse transform
  fft_c2r(c2r_in, c2r_out);

  // Normalize inverse FFT
  normalization(c2r_out, acf);
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
                      stream<real_t, yin_frame_size> &yin_frame) {
  auto d_cumsum_running = std::numeric_limits<real_t>::min();

  const auto first_acf = acf.read();

  for (counter<max_period + 1> k = 1; k < max_period + 1; ++k) {
#pragma HLS PIPELINE II = 1 rewind

    const auto dk = first_acf - acf.read() - cumsum_sq.read();

    d_cumsum_running = d_cumsum_running + dk;

    if (k > min_period - 1) {
      const auto result = dk * k / d_cumsum_running;

      yin_frame.write(result);
    }
  }
}

// y_frames: [frame_length][n_frames]
// min_period, max_period: >0, with max_period < frame_length
// returns yin: [(max_period-min_period+1)][n_frames]
void cumulative_mean_normalized_difference(
    stream<real_signal, frame_length> &y_frame_stream,
    stream<real_t, yin_frame_size> &yin_frame) {
#pragma HLS DATAFLOW

  stream<real_t, K + 1> acf;
  stream<real_t, K> cumsum_sq;

  autocorrelate_new(y_frame_stream, acf, cumsum_sq);

  diff_cmnd_output(cumsum_sq, acf, yin_frame);
}
