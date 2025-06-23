#include "stft.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <complex>
#include <string>
#include <sstream>
#include <regex>
#include <iomanip>
#include <algorithm>

// Fixed function that properly handles scientific notation
std::vector<std::vector<std::complex<double>>>
parseComplexData(const std::string &filename) {
  std::vector<std::vector<std::complex<double>>> data;
  std::ifstream file(filename);

  if (!file.is_open()) {
    std::cerr << "Error: Could not open file " << filename << std::endl;
    return data;
  }

  std::string line;
  while (std::getline(file, line)) {
    if (line.empty())
      continue;

    std::vector<std::complex<double>> rowData;
    size_t pos = 0;

    while (pos < line.length()) {
      // Find opening parenthesis
      size_t start = line.find('(', pos);
      if (start == std::string::npos)
        break;

      // Find closing parenthesis
      size_t end = line.find(')', start);
      if (end == std::string::npos)
        break;

      // Extract the content between parentheses
      std::string complexStr = line.substr(start + 1, end - start - 1);

      // Parse real and imaginary parts
      size_t jPos = complexStr.find('j');
      if (jPos != std::string::npos) {
        // Find the sign that separates real and imaginary parts
        // We need to find the + or - that's not part of scientific notation
        size_t signPos = std::string::npos;

        // Start from position 1 to skip potential leading sign of real part
        for (size_t i = 1; i < jPos; ++i) {
          if (complexStr[i] == '+' || complexStr[i] == '-') {
            // Check if this is part of scientific notation
            // Look back to see if there's an 'e' or 'E' immediately before
            if (i > 0 &&
                (complexStr[i - 1] == 'e' || complexStr[i - 1] == 'E')) {
              continue; // This is part of scientific notation, skip it
            }
            signPos = i;
            // Don't break here - we want the LAST valid sign position
            // in case there are multiple scientific notation parts
          }
        }

        double real, imag;

        if (signPos != std::string::npos) {
          // We found a separator
          std::string realStr = complexStr.substr(0, signPos);
          std::string imagStr = complexStr.substr(signPos, jPos - signPos);

          real = std::stod(realStr);
          imag = std::stod(imagStr);
        } else {
          // No separator found, might be pure imaginary or malformed
          // Check if it starts with a sign and ends with j
          if (complexStr[0] == '+' || complexStr[0] == '-') {
            // Pure imaginary number
            real = 0.0;
            imag = std::stod(complexStr.substr(0, jPos));
          } else {
            // Assume it's real part only (shouldn't happen with 'j' present)
            real = std::stod(complexStr.substr(0, jPos));
            imag = 0.0;
          }
        }

        rowData.emplace_back(real, imag);
      }

      pos = end + 1;
    }

    if (!rowData.empty()) {
      data.push_back(rowData);
    }
  }

  file.close();
  return data;
}

// Alternative regex-based approach that's more robust for scientific notation
std::vector<std::vector<std::complex<double>>>
parseComplexDataRegex(const std::string &filename) {
  std::vector<std::vector<std::complex<double>>> data;
  std::ifstream file(filename);

  if (!file.is_open()) {
    std::cerr << "Error: Could not open file " << filename << std::endl;
    return data;
  }

  std::string line;
  // Enhanced regex that properly handles scientific notation
  // Matches: (real_part+imag_partj) or (real_part-imag_partj)
  // Where real_part and imag_part can be in scientific notation
  std::regex complexRegex(
      R"(\(\s*([+-]?\d+\.?\d*(?:[eE][+-]?\d+)?)\s*([+-]\s*\d+\.?\d*(?:[eE][+-]?\d+)?)\s*j\s*\))");

  while (std::getline(file, line)) {
    if (line.empty())
      continue;

    std::vector<std::complex<double>> rowData;
    std::sregex_iterator iter(line.begin(), line.end(), complexRegex);
    std::sregex_iterator end;

    for (; iter != end; ++iter) {
      std::smatch match = *iter;

      // Extract real and imaginary parts
      std::string realStr = match[1].str();
      std::string imagStr = match[2].str();

      // Remove spaces from imaginary part
      imagStr.erase(std::remove(imagStr.begin(), imagStr.end(), ' '),
                    imagStr.end());

      try {
        double real = std::stod(realStr);
        double imag = std::stod(imagStr);
        rowData.emplace_back(real, imag);
      } catch (const std::exception &e) {
        std::cerr << "Error parsing: real='" << realStr << "', imag='"
                  << imagStr << "'" << std::endl;
        std::cerr << "Exception: " << e.what() << std::endl;
      }
    }

    if (!rowData.empty()) {
      data.push_back(rowData);
    }
  }

  file.close();
  return data;
}

// Most robust parser using manual string processing
std::vector<std::vector<std::complex<double>>>
parseComplexDataRobust(const std::string &filename) {
  std::vector<std::vector<std::complex<double>>> data;
  std::ifstream file(filename);

  if (!file.is_open()) {
    std::cerr << "Error: Could not open file " << filename << std::endl;
    return data;
  }

  std::string line;
  while (std::getline(file, line)) {
    if (line.empty())
      continue;

    std::vector<std::complex<double>> rowData;
    std::istringstream iss(line);
    std::string token;

    // Read each token (complex number in parentheses)
    while (iss >> token) {
      if (token.front() == '(' && token.back() == ')') {
        // Remove parentheses
        std::string complexStr = token.substr(1, token.length() - 2);

        // Find 'j' position
        size_t jPos = complexStr.find('j');
        if (jPos == std::string::npos)
          continue;

        // Split the string at the last +/- that's not part of scientific
        // notation
        size_t splitPos = std::string::npos;

        // Scan from right to left to find the operator that separates real and
        // imaginary parts
        for (int i = static_cast<int>(jPos) - 1; i >= 1; --i) {
          if (complexStr[i] == '+' || complexStr[i] == '-') {
            // Check if this is part of scientific notation
            if (i > 0 &&
                (complexStr[i - 1] == 'e' || complexStr[i - 1] == 'E')) {
              continue; // Skip this, it's part of scientific notation
            }
            splitPos = i;
            break;
          }
        }

        try {
          double real, imag;

          if (splitPos != std::string::npos) {
            std::string realPart = complexStr.substr(0, splitPos);
            std::string imagPart = complexStr.substr(splitPos, jPos - splitPos);

            real = std::stod(realPart);
            imag = std::stod(imagPart);
          } else {
            // Handle edge cases
            if (complexStr[0] == '+' || complexStr[0] == '-') {
              // Pure imaginary
              real = 0.0;
              imag = std::stod(complexStr.substr(0, jPos));
            } else {
              // Should not happen with valid input
              real = 0.0;
              imag = 0.0;
            }
          }

          rowData.emplace_back(real, imag);

        } catch (const std::exception &e) {
          std::cerr << "Error parsing complex number: " << complexStr
                    << std::endl;
          std::cerr << "Exception: " << e.what() << std::endl;
        }
      }
    }

    if (!rowData.empty()) {
      data.push_back(rowData);
    }
  }

  file.close();
  return data;
}

// Function to print the data for verification
void printComplexData(
    const std::vector<std::vector<std::complex<double>>> &data) {
  for (size_t i = 0; i < data.size(); ++i) {
    std::cout << "Row " << i << ": ";
    for (const auto &complex_num : data[i]) {
      std::cout << "(" << complex_num.real() << "," << complex_num.imag()
                << ") ";
    }
    std::cout << std::endl;
  }
}

int testreading() {
  // Replace with your actual filename
  std::string filename =
      "D:\\Documents\\hw_autotune\\vitis\\stft\\data\\stft.out";

  // Try the most robust parser
  auto data = parseComplexDataRobust(filename);

  std::cout << "Read " << data.size() << " rows of complex data." << std::endl;

  // Print first few rows for verification
  if (!data.empty()) {
    std::cout << "\nFirst few rows:" << std::endl;
    std::cout << std::fixed << std::setprecision(18);

    for (size_t i = 0; i < std::min(size_t(3), data.size()); ++i) {
      std::cout << "Row " << i << " (" << data[i].size()
                << " elements): " << std::endl;

      for (size_t j = 0; j < std::min(size_t(10), data[i].size()); ++j) {
        const auto &num = data[i][j];
        std::cout << "  [" << j << "]: (" << num.real() << ", " << num.imag()
                  << ")" << std::endl;
      }
      std::cout << std::endl;
    }
  }

  // Test with a specific problematic case
  std::cout << "\nTesting specific case:" << std::endl;
  std::string testLine = "(-1.10428802669048309e-01+0.00000000000000000e+00j) "
                         "(9.07433778047561646e-02-7.41797611117362976e-02j)";

  std::vector<std::complex<double>> testData;
  std::istringstream iss(testLine);
  std::string token;

  while (iss >> token) {
    if (token.front() == '(' && token.back() == ')') {
      std::string complexStr = token.substr(1, token.length() - 2);
      size_t jPos = complexStr.find('j');
      if (jPos == std::string::npos)
        continue;

      size_t splitPos = std::string::npos;
      for (int i = static_cast<int>(jPos) - 1; i >= 1; --i) {
        if (complexStr[i] == '+' || complexStr[i] == '-') {
          if (i > 0 && (complexStr[i - 1] == 'e' || complexStr[i - 1] == 'E')) {
            continue;
          }
          splitPos = i;
          break;
        }
      }

      if (splitPos != std::string::npos) {
        std::string realPart = complexStr.substr(0, splitPos);
        std::string imagPart = complexStr.substr(splitPos, jPos - splitPos);

        double real = std::stod(realPart);
        double imag = std::stod(imagPart);

        std::cout << "Parsed: " << complexStr << " -> (" << real << ", " << imag
                  << ")" << std::endl;
        testData.emplace_back(real, imag);
      }
    }
  }

  return 0;
}

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

int main() {
  std::string inputDataFile =
      "D:\\Documents\\hw_autotune\\vitis\\stft\\data\\stft_long.in";
  std::string expectedDataFile =
      "D:\\Documents\\hw_autotune\\vitis\\stft\\data\\stft_long.out";

  std::vector<double> audio = read_vector_from_file_robust(inputDataFile);
  const auto expectedData = parseComplexDataRobust(expectedDataFile);

  std::cout << expectedData.size() << " " << expectedData[0].size()
            << std::endl;

  stream<real_signal, hop_length> y;
  stream<real_signal, frame_length> y_frame;
  stream<fft_complex, frame_length> out;
  fft_exp_stream exp;

  double diff_real = 0;
  double diff_real_count = 0;
  double diff_imag = 0;
  double diff_imag_count = 0;

  size_t expected_index = 0;

  for (size_t i = 0; i < audio.size(); i += hop_length) {
    for (size_t j = 0; j < hop_length; j++) {
      y.write(audio[i + j]);
    }

    frame(y, y_frame);

    if (i / hop_length > 2) {

      stft(y_frame, out, exp);

      const auto exp_val = exp.read();

      for (size_t j = 0; j < frame_length; ++j) {
        const auto num = out.read();
        const fft_complex_scaled num_scaled = {num.real() << exp_val,
                                               num.imag() << exp_val};
        const std::complex<double> num_fp = {num_scaled.real().to_double(),
                                             num_scaled.imag().to_double()};
        if (j < frame_length / 2 + 1) {
          diff_real +=
              std::abs(num_fp.real() - expectedData[expected_index][j].real());
          diff_real_count++;
          diff_imag +=
              std::abs(num_fp.imag() - expectedData[expected_index][j].imag());
          diff_imag_count++;
        }
        // if (j < 3) {
        //   std::cout << num_scaled << " ";
        // }
      }

      // std::cout << std::endl;

      expected_index++;
    } else {
      for (size_t j = 0; j < frame_length; ++j) {
        y_frame.read();
        // std::cout << y_frame.read() << " ";
      }
    }
  }

  diff_real /= diff_real_count;
  diff_imag /= diff_imag_count;

  std::cout << "diff_real: " << diff_real
            << " diff_real_count: " << diff_real_count << std::endl;
  std::cout << "diff_imag: " << diff_imag
            << " diff_imag_count: " << diff_imag_count << std::endl;

  return 0;
}