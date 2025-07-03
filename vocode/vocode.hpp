#include "../common/common.hpp"
#include "../stft/stft.hpp"
#include "../istft/istft.hpp"

void vocode(stream<real_signal, frame_length> &y_frame,
            stream<real_t, 1> &original_f0, stream<real_t, 1> &corrected_f0,
            stream<real_t, hop_length> &audio_out);
