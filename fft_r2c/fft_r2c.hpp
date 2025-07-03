#pragma once

#include "../common/fft_common.hpp"

constexpr size_t fft_r2c_size = (1 << fft_params::max_nfft);

void fft_r2c(stream<real_t, fft_r2c_size> &in,
             stream<complex_t, fft_r2c_size> &out);

constexpr size_t fft_r2c_short_size = (1 << fft_params_short::max_nfft);

void fft_r2c_short(stream<real_t, fft_r2c_short_size> &in,
                   stream<complex_t, fft_r2c_short_size> &out);