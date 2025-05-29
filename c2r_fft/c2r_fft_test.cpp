#include "../r2c_fft/r2c_fft.hpp"
#include "c2r_fft.hpp"
#include <iostream>

int main() {
  const real_t frequency = 1000.0;
  const real_t sample_rate = 44100.0;

  r2c_real input_real_backup[r2c_n];
  r2c_real_stream input_real;
  r2c_complex_stream output_complex;
  c2r_real_stream output_real;

  for (int i = 0; i < r2c_n; i++) {
    double t = i / sample_rate;
    double sample = hls::sin(2.0 * M_PI * frequency * t);

    input_real_backup[i] = r2c_real(sample);
    input_real.write(sample);
  }

  r2c_fft_1(input_real, output_complex);
  c2r_fft_1(output_complex, output_real);

  std::cout << "FFT->IFFT Output errors:\n";
  const real_t precision = 1e-15;
  size_t count = 0;

  for (size_t i = 0; i < c2r_n; i++) {
    r2c_real input = input_real_backup[i];
    c2r_real result = output_real.read();

    auto diff = hls::abs(input - result);

    if (diff > precision) {
      count++;

      std::cout << "A[" << i << "] = " << input << " | " << result
                << " diff: " << diff << "\n";
    }
  }

  std::cout << "errors: " << count << "/" << c2r_n << "\n";

  return 0;
}