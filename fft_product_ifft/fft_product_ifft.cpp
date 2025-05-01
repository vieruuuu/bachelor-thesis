#include "fft_product_ifft.hpp"

constexpr unsigned int prod_exp = 1;

void multiply_outputs(stream<fft_complex, frame_length> &A_out,
                      stream<fft_complex, frame_length> &B_out,
                      stream<fft_complex, frame_length> &prod) {
  // alternativ, daca am overflow-uri pot incerca ce e mai jos
  // using fft_product_real =
  //     ap_fixed<fft_params::output_width * 2 + 2, 1 * 2 + 2>;
  using fft_product_real = ap_fixed<fft_params::output_width * 2, 1 * 2>;

  std::complex<fft_product_real> product;

  for (int i = 0; i < frame_length; ++i) {
#pragma HLS PIPELINE II = 1 rewind
#pragma HLS LOOP_TRIPCOUNT min = frame_length max = frame_length

    product = A_out.read();

    product *= B_out.read();

    // produs la 2 numere cu partea intreaga 1 e 2
    // deci shiftez la 1 ca sa numerele intre -1 si 1
    product.real(product.real() >> prod_exp);
    product.imag(product.imag() >> prod_exp);

    prod.write(product);
  }
}

void scale_output(stream<unsigned int, 1> &r2c_exp_A_s,
                  stream<unsigned int, 1> &r2c_exp_B_s,
                  stream<unsigned int, 1> &c2r_exp_prod_s,
                  stream<fft_real, frame_length> &ifft_result,
                  stream<real_t, frame_length> &ifft_result_normalised) {
  const int r2c_exp_A = r2c_exp_A_s.read();
  const int r2c_exp_B = r2c_exp_B_s.read();
  const int c2r_exp_prod = c2r_exp_prod_s.read();

  const int scale_factor =
      fft_params::max_nfft - c2r_exp_prod - r2c_exp_A - r2c_exp_B - prod_exp;

  fft_real_scaled result;

  for (int i = 0; i < frame_length; i++) {
#pragma HLS PIPELINE II = 1 rewind
#pragma HLS LOOP_TRIPCOUNT min = frame_length max = frame_length

    result = ifft_result.read();

    if (scale_factor > 0) {
      result >>= scale_factor;
    } else if (scale_factor < 0) {
      result <<= -scale_factor;
    }

    ifft_result_normalised.write(result);
  }
}

void fft_product_ifft(stream<fft_real, frame_length> &A_in,
                      stream<fft_real, frame_length> &B_in,
                      stream<real_t, frame_length> &ifft_result_normalised) {
#pragma HLS DATAFLOW

  stream<fft_complex, frame_length> A_out;
  stream<fft_complex, frame_length> B_out;
  stream<fft_real, frame_length> ifft_result;
  stream<fft_complex, frame_length> prod;

  stream<unsigned int, 1> r2c_exp_A_s;
  stream<unsigned int, 1> r2c_exp_B_s;
  stream<unsigned int, 1> c2r_exp_prod_s;

  fft_r2c(A_in, A_out, r2c_exp_A_s);
  fft_r2c(B_in, B_out, r2c_exp_B_s);

  multiply_outputs(A_out, B_out, prod);

  fft_c2r(prod, ifft_result, c2r_exp_prod_s);

  scale_output(r2c_exp_A_s, r2c_exp_B_s, c2r_exp_prod_s, ifft_result,
               ifft_result_normalised);
}