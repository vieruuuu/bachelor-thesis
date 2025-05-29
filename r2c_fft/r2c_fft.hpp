#pragma once

#include "../common/common.hpp"
#include "vt_fft.hpp"

constexpr size_t r2c_n = 2 * frame_length;
constexpr size_t r2c_r = 16;
constexpr size_t r2c_sample = r2c_n / r2c_r;

struct r2c_params : xf::dsp::fft::ssr_fft_default_params {
  // Scaling Mode Selection
  static const int N = r2c_n;
  static const int R = r2c_r;
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
      xf::dsp::fft::transform_direction_enum::FORWARD_TRANSFORM;
  static const xf::dsp::fft::butterfly_rnd_mode_enum butterfly_rnd_mode =
      xf::dsp::fft::butterfly_rnd_mode_enum::TRN;
};

using r2c_real = double;
using r2c_complex = complex_wrapper<r2c_real>;

using r2c_real_stream = stream<r2c_real, r2c_n>;
using r2c_complex_stream = stream<r2c_complex, r2c_n>;

using r2c_complex_stream_buffer = stream<r2c_complex, r2c_sample>[r2c_r];

void r2c_fft_1(r2c_real_stream &in, r2c_complex_stream &out);
void r2c_fft_2(r2c_real_stream &in, r2c_complex_stream &out);