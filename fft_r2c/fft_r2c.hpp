#pragma once

#include "../common/fft_common.hpp"

void fft_r2c(stream<fft_real, frame_length> &in,
             stream<fft_complex, frame_length> &out,
             stream<unsigned int, 1> &exp);