#pragma once

void build_quant_table(int quality, int table[8][8]);
void quantize_block(const double dct[8][8], const int table[8][8], int out[8][8]);
void dequantize_block(const int coeffs[8][8], const int table[8][8], double out[8][8]);
