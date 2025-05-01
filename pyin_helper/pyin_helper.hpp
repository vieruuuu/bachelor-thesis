#pragma once

#include "../boltzmann/boltzmann.hpp"
#include "../common/common.hpp"

void pyin_helper(stream<real_t, yin_frame_size> &yin_frame_stream,
                 stream<real_t, yin_frame_size> &parabolic_shifts_stream,
                 stream<real_t, 2 * n_pitch_bins> &observation_probs_stream,
                 stream<real_t, 1> &voiced_prob_stream);