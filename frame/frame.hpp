#pragma once

#include "../common/common.hpp"

void frame(stream<real_signal, hop_length> &y,
           stream<real_signal, frame_length> &y_frame);