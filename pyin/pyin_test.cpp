#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "../vocode/vocode.hpp"
#include "pyin.hpp"

bool writeVectorToFile(const std::vector<double> &data,
                       const std::string &filename,
                       int precision = 18) // Default precision updated to 18
{
  // 1. Create and open an output file stream (ofstream)
  std::ofstream outFile(filename);

  // 2. Check if the file was opened successfully
  if (!outFile.is_open()) {
    std::cerr << "Error: Could not open file '" << filename << "' for writing."
              << std::endl;
    return false; // Indicate failure
  }

  // 3. Set the output precision for floating-point numbers
  //    std::scientific forces scientific notation (e.g., 1.2345e+02).
  //    The precision value controls the number of digits *after* the decimal
  //    point. For maximum precision that guarantees round-trip conversion,
  //    consider: outFile <<
  //    std::setprecision(std::numeric_limits<double>::max_digits10);
  outFile << std::scientific
          << std::setprecision(precision); // Use scientific notation

  // 4. Iterate through the vector and write each element to the file
  for (const double &value : data) {
    outFile << value << '\n'; // Write value followed by a newline
    // Optional: Check for write errors after each write
    if (!outFile) {
      std::cerr << "Error: Failed to write to file '" << filename << "'."
                << std::endl;
      // outFile is automatically closed by its destructor when it goes out of
      // scope
      return false;
    }
  }

  // 5. File stream is automatically closed when 'outFile' goes out of scope
  // (RAII).
  //    outFile.close(); // Explicit close is optional here.

  std::cout << "Successfully wrote vector data to '" << filename
            << "' using scientific notation." << std::endl;
  return true; // Indicate success
}

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

  std::ofstream outputFile_f0(
      "D:\\Documents\\hw_autotune\\vitis\\pyin\\data\\new_new_new_viterbi_"
      "nice_fft_big.out");
  std::ofstream outputFile_f0_corrected(
      "D:\\Documents\\hw_autotune\\vitis\\pyin\\data\\new_new_new_viterbi_nice_"
      "fft"
      "corrected_big.out");

  outputFile_f0 << std::fixed << std::setprecision(5);
  outputFile_f0_corrected << std::fixed << std::setprecision(5);

  size_t f0_count = 0;

  std::vector<double> f0_vec;
  std::vector<double> f0_corrected_vec;

  for (int i = 0; i < signal.size() / hop_length; ++i) {
    stream<real_signal, hop_length> y_stream;
    stream<real_t, 1> f0;
    stream<real_t, 1> corrected_f0;

    for (int j = 0; j < hop_length; ++j) {
      y_stream.write(signal[j + i * hop_length]);
    }

    pyin(y_stream, f0, corrected_f0);
    f0_count++;

    auto f0_value = f0.read();
    auto corrected_f0_value = corrected_f0.read();

    if (f0_count < 4) {
      // ignore the first 3 values as they are bad
      continue;
    }

    f0_vec.push_back(f0_value);
    f0_corrected_vec.push_back(corrected_f0_value);

    outputFile_f0 << f0_value << '\n';
    outputFile_f0_corrected << corrected_f0_value << '\n';
  }

  auto vocoded =
      vocode(signal, f0_vec, f0_corrected_vec, sample_rate, hop_length, 5.0);

  writeVectorToFile(vocoded, "D:\\Documents\\hw_autotune\\vitis\\pyin\\data\\"
                             "new_new_new_viterbi_nice_fft_vocoded_big.out");

  std::cout << "rezultat: " << f0_count - 4 << std::endl;

  return 0;
}
