#include "cmnd.hpp"

// The length after computing the windowed differences:
constexpr int L = frame_length - win_length;

using y_frame_buffer = real_t[frame_length];
using cum_sum_buffer = real_t[frame_length];

void create_y_frame_block(
    stream<real_t, frame_length> &y_frame,
    hls::stream_of_blocks<y_frame_buffer, 2> &y_frame_block) {
  hls::write_lock<y_frame_buffer> y_frame_write_lock(y_frame_block);

  for (int i = 0; i < frame_length; ++i) {
#pragma HLS PIPELINE II = 1 rewind

    y_frame_write_lock[i] = y_frame.read();
  }
}

void create_a_b_cum(hls::stream_of_blocks<y_frame_buffer, 2> &y_frame_block,
                    stream<fft_real, frame_length> &A_stream,
                    stream<fft_real, frame_length> &B_stream,
                    hls::stream_of_blocks<cum_sum_buffer, 2> &cum_sum) {
  hls::read_lock<y_frame_buffer> y_frame_read_lock(y_frame_block);
  hls::write_lock<cum_sum_buffer> cum_sum_write_lock(cum_sum);
  // real_t previous_cum_sum_element;

  for (int i = 0; i < frame_length; ++i) {
#pragma HLS PIPELINE II = 1 rewind

    A_stream.write(y_frame_read_lock[i]);

    B_stream.write(i < win_length ? y_frame_read_lock[win_length - i] : 0.0);

    real_t cum_sum_element = y_frame_read_lock[i] * y_frame_read_lock[i];

    if (i > 0) {
      // cum_sum_element += previous_cum_sum_element;
      cum_sum_element += cum_sum_write_lock[i - 1];
    }

    cum_sum_write_lock[i] = cum_sum_element;
    // previous_cum_sum_element = cum_sum_element;
  }
}

void create_acf(stream<real_t, frame_length> &ifft_result_stream,
                stream<real_t, L> &acf) {
  for (int i = 0; i < frame_length; ++i) {
#pragma HLS PIPELINE II = 1 rewind

    const real_t ifft_result = ifft_result_stream.read();

    // ignore first [win_length] values
    if (i < win_length) {
      continue;
    }

    const real_t acf_element = hls::abs(ifft_result) < 1e-6 ? 0.0 : ifft_result;

    acf.write(acf_element);
  }
}

void create_energy(hls::stream_of_blocks<cum_sum_buffer, 2> &cum_sum,
                   stream<real_t, L> &energy) {
  hls::read_lock<cum_sum_buffer> cum_sum_read_lock(cum_sum);

  for (int i = 0; i < L; i++) {
#pragma HLS PIPELINE II = 1 rewind

    real_t energy_value;

    if (i == 0) {
      energy_value = cum_sum_read_lock[win_length] - cum_sum_read_lock[0];
    } else {
      energy_value = cum_sum_read_lock[i + win_length] - cum_sum_read_lock[i];
    }

    if (hls::abs(energy_value) < 1e-6) {
      energy_value = 0.0;
    }

    energy.write(energy_value);
  }
}

void create_yin(real_t yin[L], stream<real_t, L> &energy,
                stream<real_t, L> &acf) {
  real_t first_energy;

  for (int i = 0; i < L; i++) {
#pragma HLS PIPELINE II = 1 rewind

    const real_t energy_element = energy.read();
    const real_t acf_element = acf.read();

    if (i == 0) {
      first_energy = energy_element;
    }

    yin[i] = first_energy + energy_element - 2.0 * acf_element;
  }
}

void create_cum_yin(real_t cum_yin[max_period + 1], real_t yin[L]) {
  // === Compute cumulative mean normalization ===
  // Compute cumulative sum for tau = 1 ... max_period.
  // (tau=0 is not used in normalization)
  for (int tau = 1; tau <= max_period; tau++) {
#pragma HLS PIPELINE II = 1 rewind

    if (tau == 1) {
      cum_yin[tau] = yin[1];
    } else {
      cum_yin[tau] = cum_yin[tau - 1] + yin[tau];
    }
  }
}

void create_yin_frame(real_t cum_yin[max_period + 1], real_t yin[L],
                      stream<real_t, yin_frame_size> &yin_frame1,
                      stream<real_t, yin_frame_size> &yin_frame2) {
  // For each tau in [min_period, max_period], compute normalized value:
  // final_value = yin[tau] / ( (cumulative sum up to tau)/tau + epsilon )
  for (int tau = min_period; tau <= max_period; tau++) {
#pragma HLS PIPELINE II = 1 rewind

    const real_t cumulative_mean = cum_yin[tau] / tau;
    const real_t denom = cumulative_mean + epsilon;
    const real_t value = yin[tau] / denom;
    // Store in result; row index corresponds to (tau - min_period).

    yin_frame1.write(value);
    yin_frame2.write(value);
  }
}

void cumulative_mean_normalized_difference(
    stream<real_t, frame_length> &y_frame,
    stream<real_t, yin_frame_size> &yin_frame1,
    stream<real_t, yin_frame_size> &yin_frame2) {
#pragma HLS DATAFLOW

  hls::stream_of_blocks<y_frame_buffer, 2> y_frame_block;
  // cumulative sum of squares
  hls::stream_of_blocks<cum_sum_buffer, 2> cum_sum;

  stream<fft_real, frame_length> A_stream;
  stream<fft_real, frame_length> B_stream;
  stream<real_t, frame_length> ifft_result_stream;
  stream<real_t, L> acf;
  stream<real_t, L> energy;
  real_t yin[L] = {0.0};
  real_t cum_yin[max_period + 1] = {0.0}; // use indices 1..max_period

  create_y_frame_block(y_frame, y_frame_block);

  create_a_b_cum(y_frame_block, A_stream, B_stream, cum_sum);

  fft_product_ifft(A_stream, B_stream, ifft_result_stream);

  create_acf(ifft_result_stream, acf);

  create_energy(cum_sum, energy);

  create_yin(yin, energy, acf);

  create_cum_yin(cum_yin, yin);

  create_yin_frame(cum_yin, yin, yin_frame1, yin_frame2);
}