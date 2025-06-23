#pragma once

#include "../common/fft_common.hpp"

constexpr size_t fft_c2r_size = (1 << fft_params::max_nfft);
constexpr size_t fft_c2r_short_size = (1 << fft_params_short::max_nfft);

void fft_c2r(stream<fft_complex, fft_c2r_size> &in,
             stream<fft_real, fft_c2r_size> &out, fft_exp_stream &exp);

void fft_c2r_short(stream<fft_complex, fft_c2r_short_size> &in,
                   stream<fft_real, fft_c2r_short_size> &out,
                   fft_exp_stream &exp);