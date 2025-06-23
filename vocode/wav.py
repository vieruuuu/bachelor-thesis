import numpy as np
import librosa
import soundfile

read_file = "D:\\Documents\\hw_autotune\\vitis\\vocode\\data\\vocode.sim.out"
write_file = "D:\\Documents\\hw_autotune\\vitis\\vocode\\data\\vocode.sim.wav"

data = np.loadtxt(read_file)

soundfile.write(write_file, data, samplerate=48_000)
