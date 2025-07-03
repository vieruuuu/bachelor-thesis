#pragma once

#include "../common/common.hpp"
#include "../fft_c2r/fft_c2r.hpp"

void istft(stream<complex_t, frame_length> &stft,
           stream<real_t, hop_length> &output);