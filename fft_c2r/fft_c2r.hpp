#pragma once

#include "../common/fft_common.hpp"

void fft_c2r(stream<fft_complex, frame_length> &in,
             stream<fft_real, frame_length> &out, stream<unsigned int, 1> &exp);