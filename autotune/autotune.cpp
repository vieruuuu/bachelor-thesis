#include "autotune.hpp"

void frame(stream<real_signal, hop_length> &y,
           stream<real_signal, frame_length> &y_frame) {
  static real_signal buffer[frame_length] = {0};

  // shift_left_buffer
  for (index<frame_length - hop_length> i = 0; i < frame_length - hop_length;
       ++i) {
#pragma HLS PIPELINE II = 1
    const auto tmp = buffer[i + hop_length];

    y_frame.write(tmp);
    buffer[i] = tmp;
  }

  // read_elements
  for (index<frame_length> i = frame_length - hop_length; i < frame_length;
       ++i) {
#pragma HLS PIPELINE II = 1
    const auto tmp = y.read();

    y_frame.write(tmp);
    buffer[i] = tmp;
  }
}

void mono_to_stereo(stream<real_signal, hop_length> &input,
                    output_stream &output) {
  for (counter<hop_length> i = 0; i < hop_length; ++i) {
#pragma HLS PIPELINE II = 1 REWIND
    axis_t packet;

    packet.last = false;
    packet.data = input.read();

    output.write(packet);

    packet.last = true;

    output.write(packet);
  }
}

void stereo_to_mono(input_stream &input,
                    stream<real_signal, hop_length> &data) {
  for (counter<hop_length> i = 0; i < hop_length; ++i) {
#pragma HLS PIPELINE II = 1 REWIND

    const auto tmp = input.read();
    const auto tmpData = tmp.data;

    data.write(tmpData);

    input.read();
  }
}

void volume_down(stream<real_t, hop_length> &in,
                 stream<real_signal, hop_length> &out) {

  for (counter<hop_length> i = 0; i < hop_length; ++i) {
#pragma HLS PIPELINE II = 1 REWIND

    out.write(in.read());
  }
}

void autotune(stream<real_signal, hop_length> &y,
              stream<real_t, hop_length> &audio_out, bool next_btn,
              bool prev_btn) {
#pragma HLS DATAFLOW

  stream<real_t, 1> f0;
  stream<real_t, 1> f0_corrected;
  stream<real_signal, frame_length> y_frame;
  stream<real_signal, frame_length> y_frames[2];

  frame(y, y_frame);

  duplicate_stream<real_signal, frame_length, 2>(y_frame, y_frames);

  pyin(y_frames[0], f0, f0_corrected, next_btn, prev_btn);

  vocode(y_frames[1], f0, f0_corrected, audio_out);
}

void top(input_stream &stereo_input, output_stream &stereo_output_p,
         output_stream &stereo_output_o, bool next_btn, bool prev_btn) {
#pragma HLS interface mode = axis port = stereo_input
#pragma HLS interface mode = axis port = stereo_output_p
#pragma HLS interface mode = axis port = stereo_output_o
#pragma HLS interface mode = ap_none port = next_btn
#pragma HLS interface mode = ap_none port = prev_btn
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS DATAFLOW

  stream<real_signal, hop_length> mono_input;
  stream<real_signal, hop_length> mono_input_array[2];
  stream<real_t, hop_length> mono_output;
  stream<real_signal, hop_length> data3;

  stereo_to_mono(stereo_input, mono_input);

  duplicate_stream<real_signal, hop_length, 2>(mono_input, mono_input_array);

  autotune(mono_input_array[0], mono_output, next_btn, prev_btn);

  volume_down(mono_output, data3);

  mono_to_stereo(data3, stereo_output_p);
  mono_to_stereo(mono_input_array[1], stereo_output_o);
}
