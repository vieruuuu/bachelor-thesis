#pragma once

#include "../common/fft_common.hpp"

constexpr size_t fft_r2c_size = (1 << fft_params::max_nfft);

void fft_r2c(stream<fft_real, fft_r2c_size> &in,
             stream<fft_complex, fft_r2c_size> &out, fft_exp_stream &exp);

constexpr size_t fft_r2c_short_size = (1 << fft_params_short::max_nfft);

void fft_r2c_short(stream<fft_real, fft_r2c_short_size> &in,
                   stream<fft_complex, fft_r2c_short_size> &out,
                   fft_exp_stream &exp);