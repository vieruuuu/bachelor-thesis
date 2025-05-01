#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "cmnd.hpp"

int main() {
  std::vector<std::vector<real_t>> y_frames;

  {
    std::ifstream infile(
        "D:\\Documents\\hw_autotune\\vitis\\cmnd\\data\\y_frames.in");
    std::string line;

    while (std::getline(infile, line)) {
      std::istringstream iss(line);
      real_t number;
      std::vector<real_t> row;

      while (iss >> number) {
        row.push_back(number);
      }

      if (!row.empty()) {
        y_frames.push_back(row);
      }
    }
  }

  std::vector<std::vector<real_t>> expected_yin_frame;

  {
    std::ifstream infile(
        "D:\\Documents\\hw_autotune\\vitis\\cmnd\\data\\yin_frames.out");
    std::string line;

    while (std::getline(infile, line)) {
      std::istringstream iss(line);
      real_t number;
      std::vector<real_t> row;

      while (iss >> number) {
        row.push_back(number);
      }

      if (!row.empty()) {
        expected_yin_frame.push_back(row);
      }
    }
  }

  stream<real_t, frame_length> y_frame;
  stream<real_t, max_period - min_period + 1> yin_frame;
  stream<real_t, max_period - min_period + 1> yin_frame2;

  real_t diff_sum = 0;
  real_t diff_max = -std::numeric_limits<real_t>::infinity();
  real_t diff_min = std::numeric_limits<real_t>::infinity();
  size_t diff_count = 0;

  for (int i = 0; i < y_frames.size(); ++i) {
    for (int j = 0; j < y_frames[i].size(); ++j) {
      y_frame.write(y_frames[i][j]);
    }

    cumulative_mean_normalized_difference(y_frame, yin_frame, yin_frame2);

    for (int j = 0; j < length_of(yin_frame); j++) {
      yin_frame2.read();
      real_t result = yin_frame.read();
      real_t expected_result = expected_yin_frame[i][j];
      real_t diff = std::fabs(result - expected_result);

      // if (diff >= 1) {
      //   std::cout << "i = " << i << " result = " << result
      //             << " expected_result = " << expected_result
      //             << " diff = " << diff << '\n';
      // }

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