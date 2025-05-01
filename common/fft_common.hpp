#pragma once

#include "common_constants.generated.hpp"
#include "hls_fft.h"

struct fft_params : hls::ip_fft::params_t {
  //
  static const unsigned max_nfft = frame_length_log2; // 2048 ifft
  static const unsigned input_width = real_signal_width;
  static const unsigned output_width = real_signal_width;

  static const bool has_nfft = false;
  static const unsigned ordering_opt = hls::ip_fft::ordering::natural_order;
  // static const unsigned scaling_opt = hls::ip_fft::scaling::scaled;
  static const unsigned scaling_opt =
      hls::ip_fft::scaling::block_floating_point;
  static const unsigned rounding_opt =
      hls::ip_fft::rounding::convergent_rounding;

  static const bool ovflo = false;
  static const unsigned channels = 1;
  static const unsigned status_width = 8;
  static const unsigned config_width = 8;
  // static const unsigned config_width = 16; // cand e scaled
  static const unsigned phase_factor_width = 34;
  static const unsigned stages_block_ram = max_nfft - 9;

  //
  static const unsigned butterfly_type =
      hls::ip_fft::opt::use_mults_performance;
};

using fft_real = ap_fixed<fft_params::input_width, 1>;
using fft_complex = std::complex<fft_real>;

constexpr unsigned fft_scaled_width =
    ((fft_params::input_width + fft_params::max_nfft + 1) + 7) / 8 * 8;

using fft_real_scaled =
    ap_fixed<fft_scaled_width, fft_scaled_width - fft_params::input_width + 1>;

using fft_complex_scaled = std::complex<fft_real_scaled>;

using fft_config = hls::ip_fft::config_t<fft_params>;
using fft_config_stream = stream<fft_config, 1>;
using fft_status = hls::ip_fft::status_t<fft_params>;
using fft_status_stream = stream<fft_status, 1>;