#include "viterbi.hpp"

static const real_t log_zero = hls::log(0.0 + tiny);

inline real_t get_log_p_init(index<N> i) {
#pragma HLS inline

  static const real_t p_init_val = hls::log(1.0 / n_pitch_bins);

  if (i < n_pitch_bins) {
    return log_zero;
  }

  return p_init_val;
}

inline real_t get_log_trans(index<N> i, index<N> j) {
#pragma HLS inline

  // Determine indices for the original matrices
  const index<2> switch_row = i / n_pitch_bins;
  const index<2> switch_col = j / n_pitch_bins;
  const index<n_pitch_bins> trans_row = i % n_pitch_bins;
  const index<n_pitch_bins> trans_col = j % n_pitch_bins;

  const real_t log_t_switch_element = log_t_switch[switch_row][switch_col];

  const real_t log_transition_element = log_transition_unique_vals[
      //
      log_transition_index_map[trans_row][trans_col]
      //
  ];

  // log_transition_element can be zero
  if (!log_transition_element) {
    return log_zero;
  }

  return log_t_switch_element + log_transition_element;
}

void greedy_decode_lookahead_stream(prob_stream_t &prob_stream,
                                    path_stream &path) {

  static bool initialized = false;
  static index<N> prev_path;

  real_t best_score = -std::numeric_limits<real_t>::infinity();
  index<N> best_state = 0;

  for (index<N> s = 0; s < N; ++s) {
#pragma HLS PIPELINE II = 1

    const real_t log_trans =
        initialized ? get_log_trans(prev_path, s) : get_log_p_init(s);

    const real_t prob = prob_stream.read();

    const real_t log_prob = prob ? hls::log(prob) : log_zero;

    // score at current step
    const real_t score = log_trans + log_prob;

    // update best candidate
    if (score > best_score) {
      best_state = s;
      best_score = score;
    }
  }

  path.write(best_state);
  prev_path = best_state;

  initialized = true;
}
