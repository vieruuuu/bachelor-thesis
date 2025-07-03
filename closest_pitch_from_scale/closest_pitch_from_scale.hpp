#pragma once

#include "../common/common.hpp"
#include "../viterbi/viterbi.hpp"

void decode_state(bool next_btn, bool prev_btn, path_stream &states_stream,
                  stream<real_t, 1> &f0_stream,
                  stream<real_t, 1> &corrected_f0_stream);
