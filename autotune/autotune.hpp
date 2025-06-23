#pragma once

#include "../pyin/pyin.hpp"
#include "../stft/stft.hpp"
#include "../vocode/vocode.hpp"

#include "ap_axi_sdata.h"
#include "ap_fixed.h"
#include "ap_int.h"
#include "hls_stream.h"

using axis_t = hls::axis_data<ap_fixed<24, 1>, AXIS_ENABLE_LAST>;
using input_stream = hls::stream<axis_t>;
using output_stream = hls::stream<axis_t>;

void top(input_stream &input, output_stream &output);