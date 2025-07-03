#pragma once

#include "../common/common.hpp"
#include "../fft_r2c/fft_r2c.hpp"

void stft(stream<real_signal, frame_length> &y_frame,
          stream<complex_t, fft_r2c_short_size> &out);