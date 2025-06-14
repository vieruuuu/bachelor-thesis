#pragma once

#include "../common/common.hpp"

constexpr size_t real_greedy_lookahead = 3;
constexpr size_t greedy_lookahead = real_greedy_lookahead + 1;
constexpr size_t N = 2 * n_pitch_bins;
using prob_stream_t = hls::stream<real_t, N>;
using path_stream = hls::stream<index<N>, 1>;

using my_data_t = float; // ap_fixed<128, 20>;

constexpr size_t window_size = 4;
constexpr size_t n_states = N;

// void greedy_decode_lookahead_stream(prob_stream_t &prob_stream,
//                                     path_stream &path);

void online_windowed_viterbi(prob_stream_t &prob_stream,
                             path_stream &state_stream);