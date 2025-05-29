#include "boltzmann.hpp"

// cel mai optim
real_t boltzmann_pmf(int k, real_t lambda_param, int n) {
  const real_t exp_neg_lambda = hls::exp(-lambda_param);
  real_t numerator = hls::pow(exp_neg_lambda, (real_t)k);
  real_t Z;

  // if (hls::abs(exp_neg_lambda - 1.0) < epsilon) {
  if (hls::abs(exp_neg_lambda - 1.0) < 1e-6) {
    Z = static_cast<real_t>(n); // Sum of n ones
  } else {
    // Geometric sum from i=0 to n-1
    Z = (1.0 - hls::pow(exp_neg_lambda, (real_t)n)) / (1.0 - exp_neg_lambda);
  }

  return numerator / Z;
}

// merge bine momentan
// real_t boltzmann_pmf(int k, real_t lambda_param, int n) {
//   // Validate input parameters
//   if (k < 0 || k >= n || n <= 0) {
//     return 0.0; // Return 0 for invalid states
//   }

//   // Compute the numerator: exp(-lambda_param * k)
//   real_t numerator = hls::expf(-lambda_param * k);

//   // Compute the denominator (partition function): sum of exp(-lambda_param *
//   // i)
//   // for i=0 to n-1
//   real_t denominator = 0.0;
//   for (int i = 0; i < n; i++) {
//     denominator += hls::expf(-lambda_param * i);
//   }

//   // Return the probability mass function value
//   return numerator / denominator;
// }