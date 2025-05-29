#include "r2c_fft.hpp"
#include <iostream>

int main() {
  const double frequency = 1000.0;
  const double sample_rate = 44100.0;

  r2c_real input_real_backup[frame_length];
  r2c_real_stream input_real;
  r2c_complex_stream output_complex;

  for (int i = 0; i < frame_length; i++) {
    double t = i / sample_rate;
    double sample = hls::sin(2.0 * M_PI * frequency * t);

    input_real_backup[i] = r2c_real(sample);
    input_real.write(sample);
  }

  r2c_fft(input_real, output_complex);

  std::cout << "FFT Output:\n";
  for (size_t i = 0; i < frame_length; i++) {
    auto result = output_complex.read();

    if (i < 30) {
      std::cout << std::fixed << std::setprecision(18) << "A[" << i << "] = ("
                << result.real() << ", " << result.imag() << ")\n";
    }
  }

  return 0;
}