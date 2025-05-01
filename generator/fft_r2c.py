import numpy as np

# Parameters
frequency = 1000.0
sample_rate = 44100.0
frame_length = 2048  # Define this appropriately
nfreq = frame_length // 2 + 1

# Generate input signal
t = np.arange(frame_length) / sample_rate
input_real = np.sin(2 * np.pi * frequency * t)

# Compute FFT using numpy
fft_output = np.fft.rfft(input_real)

# Display output
print("FFT Output:")
for i, val in enumerate(fft_output):
    if i < 30 or i > nfreq - 30:
        print(f"A[{i}] = ({val.real}, {val.imag})")
