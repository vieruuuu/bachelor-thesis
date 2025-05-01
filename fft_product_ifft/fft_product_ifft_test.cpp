#include "fft_product_ifft.hpp"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <vector>

int main_big() {
  std::cout << "starting..."
            << "\n";

  constexpr int data_size = 18593;
  constexpr double tolerance = 1e-8;

  std::vector<std::vector<fft_real>> A(frame_length,
                                       std::vector<fft_real>(data_size));
  std::vector<std::vector<fft_real>> B(frame_length,
                                       std::vector<fft_real>(data_size));
  std::vector<std::vector<double>> ifft_result_expected(
      frame_length, std::vector<double>(data_size));

  stream<fft_real, frame_length> A_in;
  stream<fft_real, frame_length> B_in;
  stream<real_t, frame_length> ifft_result_normalised;

  std::ifstream A_file_in(
      "D:\\Documents\\hw_autotune\\vitis\\fft_product_ifft\\data\\a.in");
  std::ifstream B_file_in(
      "D:\\Documents\\hw_autotune\\vitis\\fft_product_ifft\\data\\b.in");
  std::ifstream ifft_result_file_in("D:\\Documents\\hw_autotune\\vitis\\fft_"
                                    "product_ifft\\data\\ifft_result.in");

  if (!A_file_in || !B_file_in || !ifft_result_file_in) {
    std::cerr << "Could not open the files" << std::endl;

    return EXIT_FAILURE;
  }

  std::cout << "starting to read files"
            << "\n";
  for (int i = 0; i < frame_length; i++) {
    for (int j = 0; j < data_size; j++) {
      A_file_in >> A[i][j];
      B_file_in >> B[i][j];
      ifft_result_file_in >> ifft_result_expected[i][j];
    }
  }

  std::cout << "files read successfully"
            << "\n";

  real_t diff_sum = 0;
  real_t diff_max = -std::numeric_limits<real_t>::infinity();
  real_t diff_min = std::numeric_limits<real_t>::infinity();
  size_t diff_count = 0;

  for (int j = 0; j < data_size; j++) {
    // std::cout << "status: " << j << "/" << data_size << "\n";
    // j = 9295;

    for (int i = 0; i < frame_length; i++) {
      A_in.write(A[i][j]);
      B_in.write(B[i][j]);
    }

    fft_product_ifft(A_in, B_in, ifft_result_normalised);

    for (size_t i = 0; i < frame_length; i++) {
      real_t result = ifft_result_normalised.read();
      real_t expected_result = ifft_result_expected[i][j];
      real_t diff = std::fabs<real_t>(result - expected_result);

      if (diff >= 1) {
        std::cout << "j = " << j << " i = " << i << " result = " << result
                  << " expected_result = " << expected_result
                  << " diff = " << diff << '\n';
      }

      if (diff > 0 && diff < diff_min) {
        diff_min = diff;
      }

      if (diff > diff_max) {
        diff_max = diff;
      }

      diff_sum += diff;
      diff_count += 1;
    }
  }

  real_t diff_avg = diff_sum / diff_count;

  std::cout << "diff_avg " << diff_avg << " diff_max " << diff_max
            << " diff_min " << diff_min << " diff_count " << diff_count << "\n";

  return EXIT_SUCCESS;
}

int main() {
  fft_real A[frame_length];
  fft_real B[frame_length];
  real_t ifft_result_expected[frame_length];

  stream<fft_real, frame_length> A_in;
  stream<fft_real, frame_length> B_in;
  stream<real_t, frame_length> ifft_result_normalised;

  std::ifstream A_file_in(
      "D:\\Documents\\hw_autotune\\vitis\\fft_product_ifft\\data\\a_small.in");
  std::ifstream B_file_in(
      "D:\\Documents\\hw_autotune\\vitis\\fft_product_ifft\\data\\b_small.in");
  std::ifstream ifft_result_file_in("D:\\Documents\\hw_autotune\\vitis\\fft_"
                                    "product_ifft\\data\\ifft_result_small.in");

  if (!A_file_in || !B_file_in || !ifft_result_file_in) {
    std::cerr << "Could not open the files" << std::endl;

    return EXIT_FAILURE;
  }

  for (int i = 0; i < frame_length; i++) {
    A_file_in >> A[i];
    B_file_in >> B[i];
    ifft_result_file_in >> ifft_result_expected[i];
  }

  double diff_sum = 0;
  double diff_max = -std::numeric_limits<double>::infinity();
  double diff_min = std::numeric_limits<double>::infinity();
  size_t diff_count = 0;

  for (int i = 0; i < frame_length; i++) {
    A_in.write(A[i]);
    B_in.write(B[i]);
  }

  fft_product_ifft(A_in, B_in, ifft_result_normalised);

  for (size_t i = 0; i < frame_length; i++) {
    real_t result = ifft_result_normalised.read();
    real_t expected_result = ifft_result_expected[i];
    real_t diff = std::fabs<real_t>(result - expected_result);

    if (diff >= 1) {
      std::cout << " i = " << i << " result = " << result
                << " expected_result = " << expected_result
                << " diff = " << diff << '\n';
    }

    if (diff > 0 && diff < diff_min) {
      diff_min = diff;
    }

    if (diff > diff_max) {
      diff_max = diff;
    }

    diff_sum += diff;
    diff_count += 1;
  }

  real_t diff_avg = diff_sum / diff_count;

  std::cout << "diff_avg " << diff_avg << " diff_max " << diff_max
            << " diff_min " << diff_min << " diff_count " << diff_count << "\n";

  return EXIT_SUCCESS;
}