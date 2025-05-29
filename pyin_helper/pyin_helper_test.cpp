

#include <cstdlib>
#include <fstream>
#include <iostream>

#include "pyin_helper.hpp"

int main() {
  stream<real_t, yin_frame_size> yin_frame;
  stream<real_t, yin_frame_size> shifts;
  stream<real_t, 1> voiced_prob_stream;
  stream<real_t, 2 * n_pitch_bins> observation_probs_stream;

  // // 9295
  // std::ifstream yin_frame_file_in(
  //     "D:\\Documents\\hw_autotune\\vitis\\pyin_helper\\data\\a1.in");
  // std::ifstream shifts_file_in(
  //     "D:\\Documents\\hw_autotune\\vitis\\pyin_helper\\data\\o1.in");

  // // 9296
  std::ifstream yin_frame_file_in(
      "D:\\Documents\\hw_autotune\\vitis\\pyin_helper\\data\\a2.in");
  std::ifstream shifts_file_in(
      "D:\\Documents\\hw_autotune\\vitis\\pyin_helper\\data\\o2.in");

  // // 9297
  // std::ifstream yin_frame_file_in(
  //     "D:\\Documents\\hw_autotune\\vitis\\pyin_helper\\data\\a3.in");
  // std::ifstream shifts_file_in(
  //     "D:\\Documents\\hw_autotune\\vitis\\pyin_helper\\data\\o3.in");

  for (int i = 0; i < yin_frame_size; ++i) {
    real_t tmp;

    yin_frame_file_in >> tmp;

    yin_frame.write(tmp);
  }

  for (int i = 0; i < yin_frame_size; ++i) {
    real_t tmp;

    shifts_file_in >> tmp;

    shifts.write(tmp);
  }

  pyin_helper(yin_frame, shifts, observation_probs_stream);

  for (int i = 0; i < 2 * n_pitch_bins; ++i) {
    std::cout << "[" << i << "] = " << observation_probs_stream.read() << " "
              << std::endl;
  }

  return EXIT_SUCCESS;
}