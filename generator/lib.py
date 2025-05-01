import numpy as np


def python_array_to_cpp(array, var_name="array", dtype="real_t", shape_str="[]"):
    array = np.asarray(array)

    def format_array(arr):
        if arr.ndim == 1:
            return "{ " + ", ".join(f"{val:.18e}" for val in arr) + " }"
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
        real = f"{val.real:.18e}"
        imag = f"{val.imag:.18e}"
        return f"{real},{imag}"

    def format_array(arr):
        if arr.ndim == 1:
            return "{ " + ", ".join(format_complex(val) for val in arr) + " }"
        else:
            return "{" + ", ".join(format_array(sub) for sub in arr) + "}"

    values_str = format_array(array)
    cpp_str = f"static {dtype} {var_name}{shape_str} = {values_str};\n"
    return cpp_str
