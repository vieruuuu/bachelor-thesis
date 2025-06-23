#pragma once

#include "../common/common.hpp"
#include "../fft_c2r/fft_c2r.hpp"

void istft(stream<fft_complex, frame_length> &stft, fft_exp_stream &exp_in,
           stream<real_t, hop_length> &output);