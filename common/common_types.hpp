#pragma once

#include "ap_fixed.h"
#include "ap_int.h"
#include "hls_dsp_builtins.h"
#include "hls_math.h"
#include "hls_stream.h"
#include "hls_streamofblocks.h"
#include <complex>
#include <limits>
#include <vector>

constexpr int real_signal_width = 24;

using real_signal = ap_fixed<real_signal_width, 1>;

using data_t = float;
using real_t = float;
using complex_t = std::complex<data_t>;

template <typename T, int DEPTH> using stream = hls::stream<T, DEPTH>;

constexpr real_t epsilon = std::numeric_limits<real_t>::epsilon();
constexpr real_t tiny = std::numeric_limits<real_t>::min();

template <typename T, int DEPTH>
constexpr inline int length_of(const stream<T, DEPTH> &s) noexcept {
  return DEPTH;
}

template <typename T, unsigned int N>
constexpr inline unsigned int length_of(const T (&arr)[N]) noexcept {
  return N;
}

constexpr size_t next_power_of_two(size_t n) {
  if (n <= 1) {
    return 1;
  }

  n -= 1;

  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  n |= n >> 32;

  n += 1;

  return n;
}

constexpr size_t log2_constexpr(size_t n) {
  size_t log = 0;

  while (n >>= 1) {
#pragma HLS loop_tripcount min = 0 max = 12 avg = 5

    log += 1;
  }

  return log;
}

constexpr size_t fitting_power_of_two(size_t n) {
  return log2_constexpr(next_power_of_two(n));
}

// ap_uint<log2_constexpr(next_power_of_two(max_length + 1))>;
template <size_t max_length> using index = size_t;

template <size_t MAX_VALUE_EXCLUSIVE>
using counter = ap_uint<fitting_power_of_two(MAX_VALUE_EXCLUSIVE + 1)>;
