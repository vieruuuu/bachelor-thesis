#pragma once

#include "../common/common.hpp"

void parabolic_interpolation(stream<real_t, yin_frame_size> &yin_frame,
                             stream<real_t, yin_frame_size> &shifts);