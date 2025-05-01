#include "fft_r2c.hpp"
#include <iostream>

int main() {
  const double frequency = 1000.0;
  const double sample_rate = 44100.0;

  fft_real input_real_backup[frame_length];
  hls::stream<fft_real, frame_length> input_real;
  hls::stream<fft_complex, frame_length> output_complex;

  stream<unsigned int, 1> exp;
  hls::stream<real_t, frame_length> output_real;

  for (int i = 0; i < frame_length; i++) {
    double t = i / sample_rate;
    double sample = hls::sin(2.0 * M_PI * frequency * t) / 1;

    input_real_backup[i] = fft_real(sample);
    input_real.write(sample);
  }

  fft_r2c(input_real, output_complex, exp);

  auto blk_exp = exp.read();

  std::cout << "FFT Output:\n";
  for (size_t i = 0; i < frame_length; i++) {
    fft_complex result = output_complex.read();
    fft_complex_scaled result_scaled = result;

    result_scaled.real(result_scaled.real() << blk_exp);
    result_scaled.imag(result_scaled.imag() << blk_exp);

    if (i < 30 || i > nfreq - 30) {
      std::cout << "A[" << i << "] = (" << result.real() << ", "
                << result.imag() << ") | (" << result_scaled.real() << ", "
                << result_scaled.imag() << ")\n";
    }
  }

  std::cout << "blk_exp " << blk_exp << "\n";

  return 0;
}