
// void greedy_decode(prob_stream_t &prob_stream, path_stream &path) {
//   static bool initialized = false;
//   static index<N> prev_path;

//   real_t best_score = -std::numeric_limits<real_t>::infinity();
//   index<N> best_state = 0;

//   for (index<N> s = 0; s < N; ++s) {
// #pragma HLS PIPELINE II = 1

//     const real_t log_trans =
//         initialized ? get_log_trans(prev_path, s) : get_log_p_init(s);

//     const real_t prob = prob_stream.read();

//     const real_t log_prob = prob ? hls::log(prob) : log_zero;

//     // score at current step
//     const real_t score = log_trans + log_prob;

//     // update best candidate
//     if (score > best_score) {
//       best_state = s;
//       best_score = score;
//     }
//   }

//   path.write(best_state);
//   prev_path = best_state;

//   initialized = true;
// }

// constexpr size_t GREEDY_LOOKAHEAD = 3;
// constexpr size_t GREEDY_WINDOW_FRAMES = GREEDY_LOOKAHEAD + 1;
// constexpr size_t GREEDY_WINDOW_SIZE = N * GREEDY_WINDOW_FRAMES;
// using GreedyWindow = SlidingWindow<real_t, N, GREEDY_WINDOW_SIZE>;

// void create_log_prob(prob_stream_t &prob_stream,
//                      prob_stream_t &log_prob_stream) {
//   for (index<N> i = 0; i < N; ++i) {
// #pragma HLS PIPELINE II = 1 rewind

//     const real_t prob = prob_stream.read();

//     const real_t log_prob = prob ? hls::log(prob) : log_zero;

//     log_prob_stream.write(log_prob);
//   }
// }

// void create_initial_state(const GreedyWindow &log_prob, index<N> &prev_state)
// {
//   real_t best_score = -std::numeric_limits<real_t>::infinity();
//   index<N> best_state = n_pitch_bins;

//   for (index<N> s = 0; s < N; ++s) {
// #pragma HLS PIPELINE II = 1 rewind

//     const real_t score = get_log_p_init(s) + log_prob.at(s);

//     if (score > best_score) {
//       best_score = score;
//       best_state = s;
//     }
//   }

//   prev_state = best_state;
// }

// void prepare_data(prob_stream_t &prob_stream, GreedyWindow &log_prob) {
// #pragma HLS DATAFLOW

//   prob_stream_t log_prob_stream;

//   create_log_prob(prob_stream, log_prob_stream);

//   log_prob.slideLeft(log_prob_stream);
// }

// void main_decoder(const GreedyWindow &log_prob, index<N> &prev_state) {
//   real_t best_score = -std::numeric_limits<real_t>::infinity();
//   index<N> best_state = n_pitch_bins;

//   // For each candidate next state, simulate lookahead
//   for (index<N> s = 0; s < N; ++s) {
// #pragma HLS PIPELINE II = 1 rewind

//     // score at current step
//     real_t score = get_log_trans(prev_state, s) + log_prob.at(s);
//     index<N> curr = s;

//     // simulate greedy future paths up to horizon
//     for (index<GREEDY_WINDOW_FRAMES> h = 1; h < GREEDY_WINDOW_FRAMES; ++h) {
//       real_t best_future = -std::numeric_limits<real_t>::infinity();
//       index<N> next_state = 0;

//       for (index<N> sp = 0; sp < N; ++sp) {
//         const real_t sc = get_log_trans(curr, sp) + log_prob.at(sp + h * N);

//         if (sc > best_future) {
//           best_future = sc;
//           next_state = sp;
//         }
//       }

//       score += best_future;
//       curr = next_state;
//     }

//     // update best candidate
//     if (score > best_score) {
//       best_score = score;
//       best_state = s;
//     }
//   }

//   prev_state = best_state;
// }

// void greedy_decode_lookahead_stream(prob_stream_t &prob_stream,
//                                     path_stream &path) {
//   static GreedyWindow log_prob(log_zero);
//   static index<N> prev_state = n_pitch_bins;
//   static bool initialized = false;

//   prepare_data(prob_stream, log_prob);

//   if (initialized) {
//     main_decoder(log_prob, prev_state);
//   } else {
//     create_initial_state(log_prob, prev_state);

//     initialized = true;
//   }

//   path.write(prev_state);
// }