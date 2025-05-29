#include "autotune.hpp"

void discard_f0(stream<real_t, 1> &f0) { f0.read(); }

void autotune(stream<real_t, hop_length> &y) {
#pragma HLS DATAFLOW

  stream<real_t, 1> f0;
  stream<real_t, 1> f0_corrected;

  pyin(y, f0, f0_corrected);

  discard_f0(f0);
}
