#pragma once

#include "../common/fft_common.hpp"

constexpr size_t fft_c2r_size = (1 << fft_params::max_nfft);
constexpr size_t fft_c2r_short_size = (1 << fft_params_short::max_nfft);

void fft_c2r(stream<complex_t, fft_c2r_size> &in,
             stream<real_t, fft_c2r_size> &out);

void fft_c2r_short(stream<complex_t, fft_c2r_short_size> &in,
                   stream<real_t, fft_c2r_short_size> &out);