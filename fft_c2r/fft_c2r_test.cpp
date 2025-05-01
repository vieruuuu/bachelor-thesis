#include "../fft_r2c/fft_r2c.hpp"
#include "fft_c2r.hpp"
#include <iostream>

int main() {
  const real_t frequency = 1000.0;
  const real_t sample_rate = 44100.0;

  fft_real input_real_backup[frame_length];
  stream<fft_real, frame_length> input_real;
  hls::stream<fft_complex, frame_length> output_complex;
  hls::stream<fft_real, frame_length> output_real;

  for (int i = 0; i < frame_length; i++) {
    double t = i / sample_rate;
    double sample = hls::sin(2.0 * M_PI * frequency * t);

    input_real_backup[i] = fft_real(sample);
    input_real.write(sample);
  }

  stream<unsigned int, 1> exp1;
  stream<unsigned int, 1> exp2;

  fft_r2c(input_real, output_complex, exp1);
  auto r2c_exp = exp1.read();

  fft_c2r(output_complex, output_real, exp2);

  auto c2r_exp = exp2.read();

  std::cout << "FFT->IFFT Output errors:\n";
  const real_t precision = 1e-8;
  size_t count = 0;

  // int scale_factor = -13;
  int scale_factor = frame_length_log2 - c2r_exp - r2c_exp;

  for (size_t i = 0; i < frame_length; i++) {
    fft_real_scaled input = input_real_backup[i];
    fft_real_scaled result = output_real.read();
    fft_real_scaled result_scaled = result;

    if (scale_factor > 0) {
      result_scaled >>= scale_factor;
    } else if (scale_factor < 0) {
      result_scaled <<= -scale_factor;
    }

    auto diff = hls::abs(input - result_scaled);

    if (diff.to_double() > precision) {
      count++;

      std::cout << "A[" << i << "] = " << input << " | " << result << " | "
                << result_scaled << " diff: " << diff << "\n";
    }
  }

  std::cout << "r2c_exp " << r2c_exp << "\n";
  std::cout << "c2r_exp " << c2r_exp << "\n";
  std::cout << "scale_factor " << scale_factor << "\n";

  std::cout << "errors: " << count << "/" << frame_length << "\n";

  return 0;
}