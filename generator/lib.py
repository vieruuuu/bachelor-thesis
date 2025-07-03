import numpy as np
import librosa
from vars import *


def python_array_to_cpp(array, var_name="array", dtype="real_t", shape_str="[]"):
    array = np.asarray(array)

    def format_array(arr):
        if arr.ndim == 1:
            return "{ " + ", ".join(f"{val:.17e}" for val in arr) + " }"
        else:
            return "{" + ",".join(format_array(sub) for sub in arr) + "}"

    values_str = format_array(array)
    cpp_str = f"inline static const {dtype} {var_name}{shape_str} = {values_str};\n"
    return cpp_str


def python_int_array_to_cpp(array, var_name="array", dtype="real_t", shape_str="[]"):
    array = np.asarray(array)

    def format_array(arr):
        if arr.ndim == 1:
            return "{ " + ", ".join(f"{val}" for val in arr) + " }"
        else:
            return "{" + ",".join(format_array(sub) for sub in arr) + "}"

    values_str = format_array(array)
    cpp_str = f"inline static const {dtype} {var_name}{shape_str} = {values_str};\n"
    return cpp_str


def python_complex_array_to_cpp(
    array, var_name="array", dtype="real_t", shape_str="[]"
):
    array = np.asarray(array)

    def format_complex(val):
        real = f"{val.real:.17e}"
        imag = f"{val.imag:.17e}"
        return f"{real},{imag}"

    def format_array(arr):
        if arr.ndim == 1:
            return "{ " + ", ".join(format_complex(val) for val in arr) + " }"
        else:
            return "{" + ", ".join(format_array(sub) for sub in arr) + "}"

    values_str = format_array(array)
    cpp_str = f"static {dtype} {var_name}{shape_str} = {values_str};\n"
    return cpp_str


def degrees_from(scale: str):
    """Return the pitch classes (degrees) that correspond to the given scale"""
    degrees = librosa.key_to_degrees(scale)
    # To properly perform pitch rounding to the nearest degree from the scale, we need to repeat
    # the first degree raised by an octave. Otherwise, pitches slightly lower than the base degree
    # would be incorrectly assigned.
    return np.concatenate((degrees, [degrees[0] + SEMITONES_IN_OCTAVE]))


def closest_pitch_from_scale_scalar(f0, scale):
    """Return the pitch closest to f0 that belongs to the given scale"""
    # Preserve nan.
    if np.isnan(f0):
        return np.nan

    degrees = degrees_from(scale)
    midi_note = librosa.hz_to_midi(f0)
    # Subtract the multiplicities of 12 so that we have the real-valued pitch class of the
    # input pitch.
    degree = midi_note % SEMITONES_IN_OCTAVE
    # Find the closest pitch class from the scale.
    degree_id = np.argmin(np.abs(degrees - degree))
    # Calculate the difference between the input pitch class and the desired pitch class.
    degree_difference = degree - degrees[degree_id]
    # Shift the input MIDI note number by the calculated difference.
    midi_note -= degree_difference
    # Convert to Hz.
    return librosa.midi_to_hz(midi_note)


def closest_pitch_from_scale(f0, scale):
    """Map each pitch in the f0 array to the closest pitch belonging to the given scale."""
    sanitized_pitch = np.zeros_like(f0)

    for i in np.arange(f0.shape[0]):
        sanitized_pitch[i] = closest_pitch_from_scale_scalar(f0[i], scale)

    return sanitized_pitch


# Python function to generate a C++ enum declaration from a list of scales
def generate_cpp_enum(scales, enum_name="Scale"):
    """
    Generate a C++ enum declaration from a list of musical scale strings.

    Args:
        scales (List[str]): List of scales in the form "Eb:min", "C#:maj", etc.
        enum_name (str): Name of the C++ enum to generate.

    Returns:
        str: A string containing the full C++ enum declaration.
    """

    def to_enum_friendly_name(scale: str) -> str:
        # Split note and quality
        note, quality = scale.split(":")
        # Replace accidentals
        note = note.replace("b", "FLAT").replace("#", "SHARP")
        # Normalize quality
        quality = quality.upper()
        # Combine and uppercase
        return f"{note}_{quality}".upper()

    # Convert all scales
    entries = [to_enum_friendly_name(s) for s in scales]
    # Join entries with commas and indentation
    body = ",\n  ".join(entries)
  
    # Build the enum declaration
    enum_lines = [f"enum {enum_name} {{", f"  {body}", f"}};"]

    enum_lines.append(f"const int {enum_name}Count = {len(entries)};")
    
    return "\n".join(enum_lines)
