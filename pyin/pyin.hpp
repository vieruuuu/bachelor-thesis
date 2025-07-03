#pragma once

#include "../boltzmann/boltzmann.hpp"
#include "../closest_pitch_from_scale/closest_pitch_from_scale.hpp"
#include "../cmnd/cmnd.hpp"
#include "../common/common.hpp"
#include "../parabolic_interpolation/parabolic_interpolation.hpp"
#include "../pyin_helper/pyin_helper.hpp"
#include "../viterbi/viterbi.hpp"

void pyin(stream<real_signal, frame_length> &y_frame,
          stream<real_t, 1> &f0_stream, stream<real_t, 1> &corrected_f0_stream,
          bool next_btn, bool prev_btn);
