import numpy as np
import librosa
import soundfile

read_file = "D:\\Documents\\hw_autotune\\vitis\\pyin\\data\\vocoded_1_ms.out"
write_file = "D:\\Documents\\hw_autotune\\vitis\\pyin\\data\\vocoded_1_ms.wav"

data = np.loadtxt(read_file)

soundfile.write(write_file, data, samplerate=48_000)
