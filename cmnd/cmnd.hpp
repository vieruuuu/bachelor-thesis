#pragma once

#include "../common/common.hpp"
#include "../fft_c2r/fft_c2r.hpp"
#include "../fft_product_ifft/fft_product_ifft.hpp"
#include "../fft_r2c/fft_r2c.hpp"

void cumulative_mean_normalized_difference(
    stream<real_t, frame_length> &y_frame,
    stream<real_t, yin_frame_size> &yin_frame1,
    stream<real_t, yin_frame_size> &yin_frame2);