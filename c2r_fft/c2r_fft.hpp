#pragma once

#include "../common/common.hpp"
#include "vt_fft.hpp"

constexpr size_t c2r_n = 2 * frame_length;
constexpr size_t c2r_r = 2;
constexpr size_t c2r_sample = c2r_n / c2r_r;

struct c2r_params : xf::dsp::fft::ssr_fft_default_params {
  // Scaling Mode Selection
  static const int N = c2r_n;
  static const int R = c2r_r;
  static const xf::dsp::fft::scaling_mode_enum scaling_mode =
      xf::dsp::fft::scaling_mode_enum::SSR_FFT_NO_SCALING;
  static const xf::dsp::fft::fft_output_order_enum output_data_order =
      xf::dsp::fft::fft_output_order_enum::SSR_FFT_NATURAL;

  //

  static const int twiddle_table_word_length = 32;
  // 2 bits are selected to store +1/-1 correctly
  static const int twiddle_table_intger_part_length = 2;

  //

  static const xf::dsp::fft::transform_direction_enum transform_direction =
      xf::dsp::fft::transform_direction_enum::REVERSE_TRANSFORM;
  static const xf::dsp::fft::butterfly_rnd_mode_enum butterfly_rnd_mode =
      xf::dsp::fft::butterfly_rnd_mode_enum::TRN;
};

using c2r_real = double;
using c2r_complex = complex_wrapper<c2r_real>;

using c2r_real_stream = stream<c2r_real, c2r_n>;
using c2r_complex_stream = stream<c2r_complex, c2r_n>;

using c2r_complex_stream_buffer = stream<c2r_complex, c2r_sample>[c2r_r];

void c2r_fft_1(c2r_complex_stream &in, c2r_real_stream &out);