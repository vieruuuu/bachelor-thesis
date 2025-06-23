#include "viterbi.hpp"

static const my_data_t log_zero = hls::log(0.0 + tiny);

inline my_data_t get_log_p_init(const index<N> i) {
#pragma HSL INLINE

  static const my_data_t p_init_val = hls::log(1.0 / n_pitch_bins);

  if (i < n_pitch_bins) {
    return log_zero;
  }

  return p_init_val;
}

inline my_data_t get_log_trans(const index<N> i, const index<N> j) {
#pragma HSL INLINE

  // Determine indices for the original matrices
  const index<2> switch_row = i / n_pitch_bins;
  const index<2> switch_col = j / n_pitch_bins;
  const index<n_pitch_bins> trans_row = i % n_pitch_bins;
  const index<n_pitch_bins> trans_col = j % n_pitch_bins;

  const auto log_t_switch_element = log_t_switch[switch_row][switch_col];

  const auto log_transition_element = log_transition_unique_vals[
      //
      log_transition_index_map[trans_row][trans_col]
      //
  ];

  // log_transition_element can be zero
  if (log_transition_element) {
    return log_t_switch_element + log_transition_element;
  }

  return log_zero;
}

inline my_data_t get_log_prob(const my_data_t prob) {
#pragma HSL INLINE

  if (prob) {
    return hls::log(prob);
  }

  return log_zero;
}

class DeltasWindow : public SlidingWindowPartitioned<my_data_t, n_states,
                                                     n_states * window_size> {
public:
  DeltasWindow() : SlidingWindowPartitioned(log_zero) {
    for (index<n_states> j = 0; j < n_states; j++) {
      window[FRAMES - 1][j] = get_log_p_init(j) + log_zero;
    }
  }
};

using PsisWindow =
    SlidingWindowPartitioned<int, n_states, n_states * window_size>;

template <size_t instances>
void drog(index<n_states> j, const DeltasWindow &new_deltas,
          stream<my_data_t, 1> &max_val_stream,
          stream<index<n_states>, 1> &max_idx_stream) {
  my_data_t max_val_array[instances];
  index<n_states> max_idx_array[instances] = {0};

  for (index<instances> c = 0; c < instances; ++c) {
#pragma HLS UNROLL

    max_val_array[c] = -std::numeric_limits<my_data_t>::infinity();
  }

  constexpr auto stages = n_states / instances;

  for (index<n_states> i = 0; i < stages; ++i) {
#pragma HLS PIPELINE II = 1 rewind

    for (index<instances> c = 0; c < instances; ++c) {
#pragma HLS UNROLL

      const auto i_real = i + c * stages;

      const auto path_val =
          get_log_trans(i_real, j) + new_deltas.at(window_size - 1, i_real);

      if (path_val > max_val_array[c]) {
        max_val_array[c] = path_val;
        max_idx_array[c] = i_real;
      }
    }
  }

  auto max_val = max_val_array[0];
  auto max_idx = max_idx_array[0];

  for (index<instances> c = 1; c < instances; ++c) {
#pragma HLS UNROLL
    if (max_val_array[c] > max_val) {
      max_val = max_val_array[c];
      max_idx = max_idx_array[c];
    }
  }

  max_val_stream.write(max_val);
  max_idx_stream.write(max_idx);
}

void ceva_after_init(prob_stream_t &prob_stream, const DeltasWindow &new_deltas,
                     const PsisWindow &new_psis,
                     stream<my_data_t, n_states> &delta_t,
                     stream<int, n_states> &psi_t, path_stream &state_stream) {
  auto maxDelta = -std::numeric_limits<my_data_t>::infinity();
  auto maxPsi = 0;

  // Calculate all possible transitions
  // For each current state j, find the most likely previous state
  for (index<n_states> j = 0; j < n_states; ++j) {
    stream<my_data_t, 1> max_val_stream;
    stream<index<n_states>, 1> max_idx_stream;

    drog<6>(j, new_deltas, max_val_stream, max_idx_stream);

    const auto currentDelta =
        max_val_stream.read() + get_log_prob(prob_stream.read());
    const auto currentPsi = max_idx_stream.read();

    if (currentDelta > maxDelta) {
      maxDelta = currentDelta;
      maxPsi = currentPsi;
    }

    delta_t.write(currentDelta);
    psi_t.write(currentPsi);
  }

  index<n_states> prev_path = maxPsi;

  // Backtrack through the window
  for (index<window_size> i = window_size - 2; i > 0; i--) {
#pragma HLS UNROLL
    prev_path = new_psis.at(i, prev_path);
  }

  // Return the state for the oldest observation
  state_stream.write(std::min(prev_path, n_pitch_bins));
}

void online_windowed_viterbi(prob_stream_t &prob_stream,
                             path_stream &state_stream) {
  // Store log probabilities
  static DeltasWindow new_deltas;
  // Store backpointers
  static PsisWindow new_psis(n_pitch_bins);

  stream<my_data_t, n_states> delta_t;
  stream<int, n_states> psi_t;

  ceva_after_init(prob_stream, new_deltas, new_psis, delta_t, psi_t,
                  state_stream);

  // Store the values
  new_deltas.slideLeft(delta_t);
  new_psis.slideLeft(psi_t);
}
