#include "fft_r2c.hpp"
#include <iostream>

int main() {
  const double frequency = 1000.0;
  const double sample_rate = 44100.0;

  real_t input_real_backup[fft_r2c_size];
  stream<real_t, fft_r2c_size> input_real;
  stream<complex_t, fft_r2c_size> output_complex;

  for (int i = 0; i < fft_r2c_size; i++) {
    double t = i / sample_rate;
    double sample = hls::sin(2.0 * M_PI * frequency * t) / 1;

    input_real_backup[i] = sample;
    input_real.write(sample);
  }

  fft_r2c(input_real, output_complex);

  std::cout << "FFT Output:\n";
  for (size_t i = 0; i < fft_r2c_size / 2 + 1; i++) {
    complex_t result = output_complex.read();

    if (i < 30 || i > (fft_r2c_size / 2 + 1) - 30) {
      std::cout << "A[" << i << "] = (" << result.real() << ", "
                << result.imag() << ")\n";
    }
  }

  return 0;
}