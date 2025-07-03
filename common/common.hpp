#pragma once

#define _GLIBCXX_ASSERTIONS

#include "common_arrays.generated.hpp"
#include "common_constants.generated.hpp"
#include "common_freq_arrays.generated.hpp"
#include "common_types.hpp"

constexpr unsigned int yin_frame_size = max_period - min_period + 1;

template <typename T, size_t FRAME_SIZE, size_t WINDOW_SIZE>
class SlidingWindow {
public:
  static constexpr size_t FRAMES = WINDOW_SIZE / FRAME_SIZE;
  static constexpr size_t SECOND_LAST_FRAME = WINDOW_SIZE - FRAME_SIZE;

  T window[FRAME_SIZE];

  SlidingWindow(const T initial_value) {
    for (index<WINDOW_SIZE> i = 0; i < WINDOW_SIZE; ++i) {
      window[i] = initial_value;
    }
  }

  inline const T at(index<WINDOW_SIZE> i) const { return window[i]; }

  inline const T frameElementAt(const index<FRAMES> frameIdx,
                                const index<FRAME_SIZE> i) const {
#pragma HLS FUNCTION_INSTANTIATE variable = frameIdx

    const index<WINDOW_SIZE> frameOffset = frameIdx * FRAME_SIZE;

    return window[i + frameOffset];
  }

  void slideLeft(stream<T, FRAME_SIZE> &in_stream) {
    for (index<FRAME_SIZE> i = 0; i < FRAME_SIZE; ++i) {
#pragma HLS PIPELINE II = 1 rewind

      for (index<FRAMES> f = 0; f < FRAMES - 1; ++f) {
#pragma HLS UNROLL
        const index<WINDOW_SIZE> curr_frame_idx = i + f * FRAME_SIZE;
        const index<WINDOW_SIZE> next_frame_idx = i + (f + 1) * FRAME_SIZE;

        const T next_element = window[next_frame_idx];

        window[curr_frame_idx] = next_element;
      }

      const index<WINDOW_SIZE> last_frame_idx = i + (FRAMES - 1) * FRAME_SIZE;

      window[last_frame_idx] = in_stream.read();
    }
  }

  void slideLeftStreaming(stream<T, FRAME_SIZE> &in_stream,
                          stream<T, WINDOW_SIZE> &out_stream) {
    for (index<WINDOW_SIZE> i = 0; i < WINDOW_SIZE; ++i) {
#pragma HLS PIPELINE II = 1 rewind
      const bool tmp_cond = i < SECOND_LAST_FRAME;
      const T tmp = tmp_cond ? window[i + FRAME_SIZE] : in_stream.read();

      out_stream.write(tmp);
      window[i] = tmp;
    }
  }
};

template <typename T, size_t FRAME_SIZE, size_t WINDOW_SIZE>
class SlidingWindowPartitioned {
public:
  static constexpr size_t FRAMES = WINDOW_SIZE / FRAME_SIZE;

  T window[FRAMES][FRAME_SIZE];

  SlidingWindowPartitioned(const T initial_value) {
    for (index<FRAME_SIZE> i = 0; i < FRAME_SIZE; ++i) {
#pragma HLS PIPELINE II = 1 rewind

      for (index<FRAMES> f = 0; f < FRAMES; ++f) {
#pragma HLS UNROLL

        window[f][i] = initial_value;
      }
    }
  }

  inline const T at(const index<FRAMES> frameIdx,
                    const index<FRAME_SIZE> i) const {
#pragma HLS FUNCTION_INSTANTIATE variable = frameIdx

    return window[frameIdx][i];
  }

  void slideLeft(stream<T, FRAME_SIZE> &in_stream) {
    for (index<FRAME_SIZE> i = 0; i < FRAME_SIZE; ++i) {
#pragma HLS PIPELINE II = 1 rewind

      for (index<FRAMES> f = 0; f < FRAMES - 1; ++f) {
#pragma HLS UNROLL
        const T next_element = window[f + 1][i];

        window[f][i] = next_element;
      }

      window[FRAMES - 1][i] = in_stream.read();
    }
  }
};

template <typename T, size_t STREAM_SIZE, size_t STREAMS>
void duplicate_stream(stream<T, STREAM_SIZE> &in_stream,
                      stream<T, STREAM_SIZE> out_streams[STREAMS]) {
  for (counter<STREAM_SIZE> i = 0; i < STREAM_SIZE; ++i) {
#pragma HLS PIPELINE II = 1 rewind

    const auto value = in_stream.read();

    for (index<STREAMS> s = 0; s < STREAMS; ++s) {
#pragma HLS UNROLL

      out_streams[s].write(value);
    }
  }
}
