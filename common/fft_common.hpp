#pragma once

#include "common_constants.generated.hpp"
#include "hls_fft.h"

struct fft_params : hls::ip_fft::params_t {
  //
  static const unsigned max_nfft = frame_length_log2 + 1;
  static const unsigned input_width = 32;
  static const unsigned output_width = 32;

  static const bool has_nfft = false;
  static const unsigned ordering_opt = hls::ip_fft::ordering::natural_order;

  static const unsigned scaling_opt =
      hls::ip_fft::scaling::block_floating_point;

  static const unsigned super_sample_rate = hls::ip_fft::ssr::ssr_1;
  static const unsigned rounding_opt =
      hls::ip_fft::rounding::convergent_rounding;

  static const bool use_native_float = false;

  static const bool ovflo = false;
  static const unsigned phase_factor_width = 25;
  static const unsigned stages_block_ram = max_nfft - 9;

  //
  static const unsigned butterfly_type = hls::ip_fft::opt::use_xtremedsp_slices;
  static const unsigned complex_mult_type =
      hls::ip_fft::opt::use_mults_performance;
};

struct fft_params_short : fft_params {
  static const unsigned max_nfft = frame_length_log2;
  static const unsigned stages_block_ram = max_nfft - 9;
};

using fft_config = hls::ip_fft::config_t<fft_params>;
using fft_config_stream = stream<fft_config, 1>;
using fft_status = hls::ip_fft::status_t<fft_params>;
using fft_status_stream = stream<fft_status, 1>;
