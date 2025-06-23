#include "vocode.hpp"
#include <cmath>     // For std::fabs
#include <fstream>   // For std::ifstream
#include <iomanip>   // For std::setprecision
#include <iostream>  // For std::cout, std::cerr
#include <limits>    // For std::numeric_limits
#include <stdexcept> // For std::runtime_error
#include <string>    // For std::string
#include <vector>    // For std::vector

std::vector<double> read_vector_from_file_robust(const std::string &filename) {
  std::vector<double> data_vector;
  std::ifstream input_file(filename);

  // Check if the file was successfully opened
  if (!input_file.is_open()) {
    throw std::runtime_error("Error: Could not open file '" + filename + "'");
  }

  std::string line;
  int line_number = 0; // Keep track for more informative error messages

  // Read the file line by line
  while (std::getline(input_file, line)) {
    line_number++;
    // Basic check for empty lines, skip them silently or add NaN if needed.
    // std::stod below will throw on empty strings anyway.
    // if (line.empty()) { continue; } // Optional: skip empty lines silently

    // Check if the line is exactly "nan"
    if (line == "nan") {
      data_vector.push_back(std::numeric_limits<double>::quiet_NaN());
    } else {
      // Try to convert the line to a double
      try {
        // std::stod attempts to parse the string as a double
        // It handles leading whitespace and scientific notation
        // (e.g., 1.23e+02)
        size_t processed_chars = 0;
        double value = std::stod(line, &processed_chars);

        // Optional: Check if the entire string was consumed.
        // std::stod stops at the first invalid character. If you want
        // to ensure the *entire* line is a valid number (ignoring maybe
        // trailing whitespace), you might add a check here.
        // For example, check if processed_chars == line.length() or if
        // remaining characters are only whitespace.
        // For simplicity here, we accept partial parses if std::stod succeeds.

        data_vector.push_back(value);
      } catch (const std::invalid_argument &ia) {
        // Catch error if the string cannot be parsed as a double
        std::cerr << "Warning: Invalid number format on line " << line_number
                  << " in file '" << filename << "'. Content: \"" << line
                  << "\". Skipping line." << std::endl;
        // Option: Add NaN instead of skipping?
        // data_vector.push_back(std::numeric_limits<double>::quiet_NaN());
      } catch (const std::out_of_range &oor) {
        // Catch error if the number is outside the range of a double
        std::cerr << "Warning: Number out of range for double on line "
                  << line_number << " in file '" << filename << "'. Content: \""
                  << line << "\". Skipping line." << std::endl;
        // Option: Add NaN instead of skipping?
        // data_vector.push_back(std::numeric_limits<double>::quiet_NaN());
      }
      // std::stod can potentially throw other exceptions, but these two are
      // primary.
    }
  }

  // input_file is automatically closed when it goes out of scope (RAII)

  return data_vector;
}

void frame(stream<real_signal, hop_length> &y,
           stream<real_signal, frame_length> &y_frame) {
  static real_signal buffer[frame_length] = {0};

  // shift_left_buffer
  for (int i = 0; i < frame_length - hop_length; ++i) {
#pragma HLS PIPELINE II = 1 rewind
    const auto tmp = buffer[i + hop_length];

    y_frame.write(tmp);
    buffer[i] = tmp;
  }

  // read_elements
  for (int i = frame_length - hop_length; i < frame_length; ++i) {
#pragma HLS PIPELINE II = 1 rewind
    const auto tmp = y.read();

    y_frame.write(tmp);
    buffer[i] = tmp;
  }
}

// int main2() {
//   // --- Configuration ---
//   const std::string audio_file =
//       "D:\\Documents\\hw_autotune\\vitis\\vocode\\data\\vocode.in";
//   const std::string original_f0_file =
//       "D:\\Documents\\hw_autotune\\vitis\\vocode\\data\\vocode_f0.in";
//   const std::string corrected_f0_file =
//       "D:\\Documents\\hw_autotune\\vitis\\vocode\\data\\vocode_corrected_f0.in";
//   const std::string expected_output_file =
//       "D:\\Documents\\hw_autotune\\vitis\\vocode\\data\\vocode.out";

//   std::vector<double> audio = read_vector_from_file_robust(audio_file);
//   std::vector<double> original_f0 =
//       read_vector_from_file_robust(original_f0_file);
//   std::vector<double> corrected_f0 =
//       read_vector_from_file_robust(corrected_f0_file);

//   size_t real_index = 0;

//   for (size_t i = 0; i < audio.size(); i += hop_length) {
//     stream<real_signal, hop_length> y;
//     stream<real_signal, frame_length> y_frame;
//     stream<real_t, 1> original_f0_stream;
//     stream<real_t, 1> corrected_f0_stream;
//     stream<fft_complex, fft_r2c_short_size> S_shifted;
//     fft_exp_stream exp_new;

//     for (size_t f = 0; f < hop_length; ++f) {
//       y.write(audio[i + f]);
//     }

//     frame(y, y_frame);

//     if (i / hop_length > 2) {
//       original_f0_stream.write(original_f0[real_index]);
//       corrected_f0_stream.write(corrected_f0[real_index]);

//       vocode(y_frame, original_f0_stream, corrected_f0_stream, S_shifted,
//              exp_new);

//       const auto eeee = exp_new.read();

//       std::cout << real_index + 1 << "\tscale:" << eeee << std::endl;
//       const auto eaeaea = real_t(1 << eeee);
//       for (size_t j = 0; j < frame_length; ++j) {
//         const auto element = S_shifted.read();
//         const auto element_scaled =
//             std::complex<real_t>(element.real().to_float() * eaeaea,
//                                  element.imag().to_float() * eaeaea);

//         if (j > 4 && j < 10) {
//           std::cout << element_scaled << "\t";
//         }
//       }

//       std::cout << std::endl << std::endl;

//       real_index++;
//     } else {
//       for (size_t j = 0; j < frame_length; ++j) {
//         y_frame.read();
//         // std::cout << y_frame.read() << " ";
//       }
//     }
//   }
// }

int main() {
  // --- Configuration ---
  const std::string audio_file =
      "D:\\Documents\\hw_autotune\\vitis\\vocode\\data\\vocode_long.in";
  const std::string original_f0_file =
      "D:\\Documents\\hw_autotune\\vitis\\vocode\\data\\vocode_f0_long.in";
  const std::string corrected_f0_file =
      "D:\\Documents\\hw_autotune\\vitis\\vocode\\data\\vocode_corrected_f0_"
      "long.in";
  const std::string output_file =
      "D:\\Documents\\hw_autotune\\vitis\\vocode\\data\\vocode.sim.out";

  std::ofstream outFile(output_file);

  std::vector<double> audio = read_vector_from_file_robust(audio_file);
  std::vector<double> original_f0 =
      read_vector_from_file_robust(original_f0_file);
  std::vector<double> corrected_f0 =
      read_vector_from_file_robust(corrected_f0_file);

  size_t real_index = 0;

  for (size_t i = 0; i < audio.size(); i += hop_length) {
    stream<real_t, hop_length> out;
    stream<real_signal, hop_length> y;
    stream<real_signal, frame_length> y_frame;
    stream<real_t, 1> original_f0_stream;
    stream<real_t, 1> corrected_f0_stream;

    for (size_t f = 0; f < hop_length; ++f) {
      y.write(audio[i + f]);
    }

    frame(y, y_frame);

    if (i / hop_length > 2) {
      original_f0_stream.write(original_f0[real_index]);
      corrected_f0_stream.write(corrected_f0[real_index]);

      vocode(y_frame, original_f0_stream, corrected_f0_stream, out);

      for (size_t j = 0; j < hop_length; ++j) {
        const auto element = out.read();

        outFile << std::scientific << std::setprecision(18) << element << '\n';
      }

    } else {
      for (size_t j = 0; j < frame_length; ++j) {
        y_frame.read();
        // std::cout << y_frame.read() << " ";
      }
    }
  }
}
