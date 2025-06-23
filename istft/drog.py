@jit(nopython=True, cache=True)
def __overlap_add(y, ytmp, hop_length):
    # numba-accelerated overlap add for inverse stft
    # y is the pre-allocated output buffer
    # ytmp is the windowed inverse-stft frames
    # hop_length is the hop-length of the STFT analysis

    n_fft = ytmp.shape[-2]
    N = n_fft
    for frame in range(ytmp.shape[-1]):
        sample = frame * hop_length
        if N > y.shape[-1] - sample:
            N = y.shape[-1] - sample

        y[..., sample : (sample + N)] += ytmp[..., :N, frame]



def window_sumsquare(
    *,
    window: _WindowSpec,
    n_frames: int,
    hop_length: int = 512,
    win_length: Optional[int] = None,
    n_fft: int = 2048,
    dtype: DTypeLike = np.float32,
    norm: Optional[float] = None,
) -> np.ndarray:
    """Compute the sum-square envelope of a window function at a given hop length.

    This is used to estimate modulation effects induced by windowing observations
    in short-time Fourier transforms.

    Parameters
    ----------
    window : string, tuple, number, callable, or list-like
        Window specification, as in `get_window`
    n_frames : int > 0
        The number of analysis frames
    hop_length : int > 0
        The number of samples to advance between frames
    win_length : [optional]
        The length of the window function.  By default, this matches ``n_fft``.
    n_fft : int > 0
        The length of each analysis frame.
    dtype : np.dtype
        The data type of the output
    norm : {np.inf, -np.inf, 0, float > 0, None}
        Normalization mode used in window construction.
        Note that this does not affect the squaring operation.

    Returns
    -------
    wss : np.ndarray, shape=``(n_fft + hop_length * (n_frames - 1))``
        The sum-squared envelope of the window function

    Examples
    --------
    For a fixed frame length (2048), compare modulation effects for a Hann window
    at different hop lengths:

    >>> n_frames = 50
    >>> wss_256 = librosa.filters.window_sumsquare(window='hann', n_frames=n_frames, hop_length=256)
    >>> wss_512 = librosa.filters.window_sumsquare(window='hann', n_frames=n_frames, hop_length=512)
    >>> wss_1024 = librosa.filters.window_sumsquare(window='hann', n_frames=n_frames, hop_length=1024)

    >>> import matplotlib.pyplot as plt
    >>> fig, ax = plt.subplots(nrows=3, sharey=True)
    >>> ax[0].plot(wss_256)
    >>> ax[0].set(title='hop_length=256')
    >>> ax[1].plot(wss_512)
    >>> ax[1].set(title='hop_length=512')
    >>> ax[2].plot(wss_1024)
    >>> ax[2].set(title='hop_length=1024')
    """
    if win_length is None:
        win_length = n_fft

    n = n_fft + hop_length * (n_frames - 1)
    x = np.zeros(n, dtype=dtype)

    # Compute the squared window at the desired length
    win_sq = get_window(window, win_length)
    win_sq = util.normalize(win_sq, norm=norm) ** 2
    win_sq = util.pad_center(win_sq, size=n_fft)

    # Fill the envelope
    __window_ss_fill(x, win_sq, n_frames, hop_length)

    return x



@jit(nopython=True, cache=True)
def __window_ss_fill(x, win_sq, n_frames, hop_length):  # pragma: no cover
    """Compute the sum-square envelope of a window."""
    n = len(x)
    n_fft = len(win_sq)
    for i in range(n_frames):
        sample = i * hop_length
        x[sample : min(n, sample + n_fft)] += win_sq[: max(0, min(n_fft, n - sample))]



def istft(
    stft_matrix: np.ndarray,
    *,
    hop_length: Optional[int] = None,
    win_length: Optional[int] = None,
    n_fft: Optional[int] = None,
    window: _WindowSpec = "hann",
    center: bool = True,
    dtype: Optional[DTypeLike] = None,
    length: Optional[int] = None,
    out: Optional[np.ndarray] = None,
) -> np.ndarray:
    ifft_window = get_window(window, win_length, fftbins=True)
 
    shape = list(stft_matrix.shape[:-2])
    expected_signal_len = n_fft + hop_length * (n_frames - 1)

    y = np.zeros(shape, dtype=dtype)

    start_frame = 0
    offset = 0

    frame = 0
    # invert the block and apply the window function
    ytmp = ifft_window * fft.irfft(stft_matrix[..., bl_s:bl_t], n=n_fft, axis=-2)

    # Overlap-add the istft block starting at the i'th frame
    __overlap_add(y[..., frame * hop_length + offset :], ytmp, hop_length)

    frame += bl_t - bl_s

    # Normalize by sum of squared window
    ifft_window_sum = window_sumsquare(
        window=window,
        n_frames=n_frames,
        win_length=win_length,
        n_fft=n_fft,
        hop_length=hop_length,
        dtype=dtype,
    )
 
    start = 0

    ifft_window_sum = util.fix_length(ifft_window_sum[..., start:], size=y.shape[-1])

    approx_nonzero_indices = ifft_window_sum > util.tiny(ifft_window_sum)

    y[..., approx_nonzero_indices] /= ifft_window_sum[approx_nonzero_indices]

    return y
