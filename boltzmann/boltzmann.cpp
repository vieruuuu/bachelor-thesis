#include "boltzmann.hpp"

real_t boltzmann_pmf(const int k, const int n) {
  static const auto exp_neg_lambda = hls::exp(-boltzmann_parameter);
  static const auto z_type = hls::abs(exp_neg_lambda - 1.0) < 1e-6;

  const auto numerator = hls::pow(exp_neg_lambda, (real_t)k);
  real_t Z;

  // if (hls::abs(exp_neg_lambda - 1.0) < epsilon) {
  if (z_type) {
    Z = static_cast<real_t>(n); // Sum of n ones
  } else {
    // Geometric sum from i=0 to n-1
    Z = (1.0 - hls::pow(exp_neg_lambda, (real_t)n)) / (1.0 - exp_neg_lambda);
  }

  return numerator / Z;
}
