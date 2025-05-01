// std::vector<int> _viterbi(const std::vector<std::vector<double>> &log_prob,
//                           const std::vector<std::vector<double>> &log_trans,
//                           const std::vector<double> &log_p_init) {
//   int n_steps = log_prob.size();
//   int n_states = log_prob[0].size();

//   std::vector<int> state(n_steps, 0);
//   std::vector<std::vector<double>> value(n_steps,
//                                          std::vector<double>(n_states, 0.0));
//   std::vector<std::vector<int>> ptr(n_steps, std::vector<int>(n_states, 0));

//   // Factor in initial state distribution
//   for (int j = 0; j < n_states; ++j) {
//     value[0][j] = log_prob[0][j] + log_p_init[j];
//   }

//   for (int t = 1; t < n_steps; ++t) {
//     // For each state j
//     for (int j = 0; j < n_states; ++j) {
//       // Find the most likely previous state
//       double max_val = -std::numeric_limits<double>::infinity();
//       int max_state = 0;

//       for (int k = 0; k < n_states; ++k) {
//         double trans_out = value[t - 1][k] + log_trans[k][j];
//         if (trans_out > max_val) {
//           max_val = trans_out;
//           max_state = k;
//         }
//       }

//       ptr[t][j] = max_state;
//       value[t][j] = log_prob[t][j] + max_val;
//     }
//   }

//   // Roll backward to find the optimal path
//   // Get the last state
//   double max_final = -std::numeric_limits<double>::infinity();
//   for (int j = 0; j < n_states; ++j) {
//     if (value[n_steps - 1][j] > max_final) {
//       max_final = value[n_steps - 1][j];
//       state[n_steps - 1] = j;
//     }
//   }

//   // Backtrack
//   for (int t = n_steps - 2; t >= 0; --t) {
//     state[t] = ptr[t + 1][state[t + 1]];
//   }

//   return state;
// }

// /**
//  * Viterbi decoding from observation likelihoods.
//  *
//  * @param prob Observation probabilities [n_steps][n_states]
//  * @return Either states or states
//  */
// std::vector<int> viterbi(const std::vector<std::vector<double>> &prob) {
//   int n_steps = prob.size();
//   int n_states = prob[0].size();

//   // Compute log probabilities
//   std::vector<std::vector<double>> log_trans(n_states,
//                                              std::vector<double>(n_states));
//   for (int i = 0; i < n_states; ++i) {
//     for (int j = 0; j < n_states; ++j) {
//       log_trans[i][j] = get_log_trans(i, j);
//     }
//   }

//   std::vector<std::vector<double>> log_prob(n_steps,
//                                             std::vector<double>(n_states));
//   for (int t = 0; t < n_steps; ++t) {
//     for (int s = 0; s < n_states; ++s) {
//       log_prob[t][s] = std::log(prob[t][s] + tiny);
//     }
//   }

//   std::vector<double> log_p_init(n_states);
//   for (int i = 0; i < n_states; ++i) {
//     log_p_init[i] = get_log_p_init(i);
//   }

//   // Run Viterbi algorithm
//   return _viterbi(log_prob, log_trans, log_p_init);
// }