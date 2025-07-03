#include "../fft_r2c/fft_r2c.hpp"
#include "fft_c2r.hpp"
#include <iostream>

int main() {
  const real_t frequency = 1000.0;
  const real_t sample_rate = 44100.0;

  real_t input_real_backup[fft_r2c_size];
  stream<real_t, fft_r2c_size> input_real;
  hls::stream<complex_t, fft_r2c_size> output_complex;
  hls::stream<real_t, fft_r2c_size> output_real;

  for (int i = 0; i < fft_r2c_size; i++) {
    double t = i / sample_rate;
    double sample = hls::sin(2.0 * M_PI * frequency * t);

    input_real_backup[i] = sample;
    input_real.write(sample);
  }

  fft_r2c(input_real, output_complex);

  fft_c2r(output_complex, output_real);

  std::cout << "FFT->IFFT Output errors:\n";
  const real_t precision = 1e-8;
  size_t count = 0;

  for (size_t i = 0; i < fft_r2c_size; i++) {
    real_t input = input_real_backup[i];
    real_t result = output_real.read() / fft_c2r_size;

    auto diff = hls::abs(input - result);

    if (diff > precision) {
      count++;

      std::cout << "A[" << i << "] = " << input << " | " << result
                << " diff: " << diff << "\n";
    }
  }

  std::cout << "errors: " << count << "/" << fft_r2c_size << "\n";

  return 0;
}