#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "pyin.hpp"

int main() {
  // Read data from file
  std::ifstream infile(
      "D:\\Documents\\hw_autotune\\vitis\\pyin\\data\\blue_pill_long.in");

  if (!infile) {
    std::cerr << "Could not open the file data.txt." << std::endl;
    return 1;
  }

  t_data_2d signal;
  t_data value;
  int i = 0;

  while (infile >> value) {
    i++;
    signal.push_back(value);
  }

  std::cout << "am citit: " << i << std::endl;

  std::ofstream outputFile(
      "D:\\Documents\\hw_autotune\\vitis\\pyin\\data\\greedy_3_long.out");

  outputFile << std::fixed << std::setprecision(5);

  size_t f0_count = 0;

  for (int i = 0; i < signal.size() / hop_length; ++i) {
    stream<real_t, hop_length> y_stream;
    stream<real_t, 1> f0;

    for (int j = 0; j < hop_length; ++j) {
      y_stream.write(signal[j + i * hop_length]);
    }

    pyin(y_stream, f0);

    outputFile << f0.read() << '\n';
    f0_count++;
  }

  std::cout << "rezultat: " << f0_count << std::endl;

  return 0;
}
