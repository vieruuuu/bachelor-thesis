#include "../common/common.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <numeric>
#include <vector>

std::vector<double> vocode(const std::vector<double> &audio,
                           const std::vector<double> &original_f0,
                           const std::vector<double> &target_f0,
                           int sample_rate, int hop_length,
                           double fade_duration_ms);