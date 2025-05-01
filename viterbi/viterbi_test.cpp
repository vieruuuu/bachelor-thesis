#include "viterbi.hpp"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

int main() {
  std::vector<std::vector<real_t>> prob;

  {
    std::ifstream infile(
        "D:\\Documents\\hw_autotune\\vitis\\viterbi\\data\\viterbi_big.in");
    std::string line;

    while (std::getline(infile, line)) {
      std::istringstream iss(line);
      real_t number;
      std::vector<real_t> row;

      while (iss >> number) {
        row.push_back(number);
      }

      if (!row.empty()) {
        prob.push_back(row);
      }
    }
  }

  std::ofstream outFile(
      "D:\\Documents\\hw_autotune\\vitis\\viterbi\\data\\greedy_stream.out");

  for (int i = 0; i < prob.size(); ++i) {
    prob_stream_t prob_s;
    path_stream state;

    for (int j = 0; j < prob[i].size(); ++j) {
      prob_s.write(prob[i][j]);
    }

    greedy_decode_lookahead_stream(prob_s, state);

    outFile << state.read() << std::endl;
  }

  // auto result = greedy_decode_lookahead(prob, 3);

  // std::ofstream outFile(
  //     "D:\\Documents\\hw_autotune\\vitis\\viterbi\\data\\greedy_3.out");

  // for (int i = 0; i < result.size(); ++i) {
  //   outFile << result[i] << std::endl;
  // }

  return 0;
}