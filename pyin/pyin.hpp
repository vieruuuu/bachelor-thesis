#pragma once

#include "../boltzmann/boltzmann.hpp"
#include "../cmnd/cmnd.hpp"
#include "../common/common.hpp"
#include "../frame/frame.hpp"
#include "../parabolic_interpolation/parabolic_interpolation.hpp"
#include "../pyin_helper/pyin_helper.hpp"
#include "../viterbi/viterbi.hpp"

void pyin(stream<real_t, hop_length> &y, stream<real_t, 1> &f0);