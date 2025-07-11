import numpy as np
import librosa
import soundfile

read_file = "D:\\Documents\\hw_autotune\\vitis\\istft\\data\\istft_tb.out"
write_file = "D:\\Documents\\hw_autotune\\vitis\\istft\\data\\istft_tb.wav"

data = np.loadtxt(read_file)

soundfile.write(write_file, data, samplerate=48_000)
