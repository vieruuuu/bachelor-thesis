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

/**
 * @brief Compares two vectors of doubles for approximate equality.
 *
 * @param vec1 The first vector.
 * @param vec2 The second vector.
 * @param tolerance The maximum allowed absolute difference between
 * corresponding elements.
 * @return true if the vectors are approximately equal, false otherwise.
 */
bool compare_vectors(const std::vector<double> &vec1,
                     const std::vector<double> &vec2, double tolerance) {
  if (vec1.size() != vec2.size()) {
    std::cerr << "Error: Vector sizes differ (" << vec1.size() << " vs "
              << vec2.size() << ")" << std::endl;
    return false;
  }

  size_t errors = 0;
  size_t non_zero = 0;

  for (size_t i = 0; i < vec1.size(); ++i) {
    if (vec2[i] != 0) {
      non_zero++;
    }

    if (std::fabs(vec1[i] - vec2[i]) > tolerance) {
      errors++;
      if (errors < 100) {
        std::cerr << "Error: Mismatch at index " << i << ". "
                  << "Expected: " << std::fixed << std::setprecision(10)
                  << vec2[i] << ", Got: " << std::fixed << std::setprecision(10)
                  << vec1[i] << ", Difference: " << std::fabs(vec1[i] - vec2[i])
                  << std::endl;
      }
      // return false;
    }
  }

  std::cerr << errors << "/" << non_zero << std::endl;

  return true;
}

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
  // --- Configuration ---
  const std::string audio_file =
      "D:\\Documents\\hw_autotune\\vitis\\vocode\\data\\audio.in";
  const std::string original_f0_file =
      "D:\\Documents\\hw_autotune\\vitis\\vocode\\data\\original_f0.in";
  const std::string target_f0_file =
      "D:\\Documents\\hw_autotune\\vitis\\vocode\\data\\target_f0.in";
  const std::string expected_output_file =
      "D:\\Documents\\hw_autotune\\vitis\\vocode\\data\\audio.out";
  std::string outputFilename =
      "D:\\Documents\\hw_autotune\\vitis\\vocode\\data\\audio_tb.out";

  // Define sample rate and hop length (adjust as needed for your vocode
  // implementation)
  const double fade_duration_ms = 1.0; // Default or specific value

  // Tolerance for floating-point comparisons
  const double tolerance = 1e-6; // Adjust if necessary

  std::cout << "--- Starting Vocode Test ---" << std::endl;
  std::cout << "Sample Rate: " << sample_rate << std::endl;
  std::cout << "Hop Length: " << hop_length << std::endl;
  std::cout << "Fade Duration (ms): " << fade_duration_ms << std::endl;
  std::cout << "Comparison Tolerance: " << tolerance << std::endl;

  try {
    // --- Load Input Data ---
    std::cout << "Loading audio data from: " << audio_file << std::endl;
    std::vector<double> audio = read_vector_from_file_robust(audio_file);
    std::cout << " -> Loaded " << audio.size() << " samples." << std::endl;

    std::cout << "Loading original F0 data from: " << original_f0_file
              << std::endl;
    std::vector<double> original_f0 =
        read_vector_from_file_robust(original_f0_file);
    std::cout << " -> Loaded " << original_f0.size() << " F0 values."
              << std::endl;

    std::cout << "Loading target F0 data from: " << target_f0_file << std::endl;
    std::vector<double> target_f0 =
        read_vector_from_file_robust(target_f0_file);

    for (int i = 0; i < original_f0.size(); ++i) {
      std::cout << original_f0[i] << " " << target_f0[i] << std::endl;
    }

    std::cout << " -> Loaded " << target_f0.size() << " F0 values."
              << std::endl;

    // --- Load Expected Output ---
    std::cout << "Loading expected output data from: " << expected_output_file
              << std::endl;
    std::vector<double> expected_output =
        read_vector_from_file_robust(expected_output_file);
    std::cout << " -> Loaded " << expected_output.size() << " expected samples."
              << std::endl;

    // --- Run Vocode Function ---
    std::cout << "Running vocode function..." << std::endl;
    std::vector<double> actual_output =
        vocode(audio, original_f0, target_f0, sample_rate, hop_length,
               fade_duration_ms);
    std::cout << " -> Vocode function finished. Output size: "
              << actual_output.size() << " samples." << std::endl;

    if (writeVectorToFile(actual_output, outputFilename)) {
      std::cout << "File write operation successful." << std::endl;
    } else {
      std::cout << "File write operation failed." << std::endl;
      return 1; // Indicate error in main
    }

    // --- Compare Results ---
    std::cout << "Comparing actual output with expected output..." << std::endl;
    bool success = compare_vectors(actual_output, expected_output, tolerance);

    if (success) {
      std::cout << "--- Test PASSED ---" << std::endl;
      return 0; // Indicate success
    } else {
      std::cerr << "--- Test FAILED ---" << std::endl;
      return 1; // Indicate failure
    }

  } catch (const std::runtime_error &e) {
    std::cerr << "Runtime Error: " << e.what() << std::endl;
    std::cerr << "--- Test FAILED ---" << std::endl;
    return 1; // Indicate failure due to error
  } catch (const std::exception &e) {
    std::cerr << "Standard Exception: " << e.what() << std::endl;
    std::cerr << "--- Test FAILED ---" << std::endl;
    return 1; // Indicate failure due to error
  } catch (...) {
    std::cerr << "An unknown error occurred." << std::endl;
    std::cerr << "--- Test FAILED ---" << std::endl;
    return 1; // Indicate failure due to error
  }
}
