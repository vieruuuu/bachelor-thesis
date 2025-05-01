

#include <cstdlib>
#include <fstream>
#include <iostream>

#include "parabolic_interpolation.hpp"

int main() {
  stream<real_t, yin_frame_size> yin_frame;
  stream<real_t, yin_frame_size> shifts;

  std::ifstream a1_file_in("D:\\Documents\\hw_autotune\\vitis\\parabolic_"
                           "interpolation\\data\\a1.in");
  std::ifstream a2_file_in("D:\\Documents\\hw_autotune\\vitis\\parabolic_"
                           "interpolation\\data\\a2.in");
  std::ifstream a3_file_in("D:\\Documents\\hw_autotune\\vitis\\parabolic_"
                           "interpolation\\data\\a3.in");

  for (int i = 0; i < yin_frame_size; ++i) {
    real_t tmp;

    a1_file_in >> tmp;

    yin_frame.write(tmp);
  }

  // for (int i = 0; i < yin_frame_size; ++i) {
  //   real_t tmp;

  //   a2_file_in >> tmp;

  //   yin_frame.write(tmp);
  // }
  // for (int i = 0; i < yin_frame_size; ++i) {
  //   real_t tmp;

  //   a3_file_in >> tmp;

  //   yin_frame.write(tmp);
  // }

  parabolic_interpolation(yin_frame, shifts);

  for (int i = 0; i < yin_frame_size; ++i) {
    std::cout << "[" << i << "] = " << shifts.read() << "\n";
  }

  return EXIT_SUCCESS;
}