#pragma once

#include "../common/common.hpp"

#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>
#include <cmath>
#include <functional>
#include <optional>

constexpr size_t real_greedy_lookahead = 3;
constexpr size_t greedy_lookahead = real_greedy_lookahead + 1;
constexpr size_t N = 2 * n_pitch_bins;
using prob_stream_t = hls::stream<real_t, N>;
using path_stream = hls::stream<index<N>, 1>;

void greedy_decode_lookahead_stream(prob_stream_t &prob_stream,
                                    path_stream &path);

void online_windowed_viterbi(prob_stream_t &prob_stream,
                             path_stream &state_stream);