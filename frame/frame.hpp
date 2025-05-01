#pragma once

#include "../common/common.hpp"

void frame(stream<real_t, hop_length> &y,
           stream<real_t, frame_length> &y_frame);