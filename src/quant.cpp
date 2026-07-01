#include "quant.h"

#include <cmath>

namespace {

const int kBaseLumaTable[8][8] = {
    {16, 11, 10, 16, 24, 40, 51, 61},
    {12, 12, 14, 19, 26, 58, 60, 55},
    {14, 13, 16, 24, 40, 57, 69, 56},
    {14, 17, 22, 29, 51, 87, 80, 62},
    {18, 22, 37, 56, 68, 109, 103, 77},
    {24, 35, 55, 64, 81, 104, 113, 92},
    {49, 64, 78, 87, 103, 121, 120, 101},
    {72, 92, 95, 98, 112, 100, 103, 99},
};

int clamp_byte(int v) {
    if (v < 1) return 1;
    if (v > 255) return 255;
    return v;
}

}  // namespace

void build_quant_table(int quality, int table[8][8]) {
    if (quality < 1) quality = 1;
    if (quality > 100) quality = 100;

    int scale = quality < 50 ? (5000 / quality) : (200 - quality * 2);

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            int base = kBaseLumaTable[y][x];
            table[y][x] = clamp_byte((base * scale + 50) / 100);
        }
    }
}

void quantize_block(const double dct[8][8], const int table[8][8], int out[8][8]) {
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            out[y][x] = static_cast<int>(std::round(dct[y][x] / table[y][x]));
        }
    }
}

void dequantize_block(const int coeffs[8][8], const int table[8][8], double out[8][8]) {
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            out[y][x] = static_cast<double>(coeffs[y][x] * table[y][x]);
        }
    }
}
