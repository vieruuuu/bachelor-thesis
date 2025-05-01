#pragma once

#include "../fft_c2r/fft_c2r.hpp"
#include "../fft_r2c/fft_r2c.hpp"

void fft_product_ifft(stream<fft_real, frame_length> &A_in,
                      stream<fft_real, frame_length> &B_in,
                      stream<real_t, frame_length> &ifft_result_normalised);