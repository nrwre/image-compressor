#include "dct.h"

#include <cmath>

namespace {

constexpr double kPi = 3.14159265358979323846;

double basis[8][8];
double alpha[8];
bool tables_ready = false;

void build_tables() {
    if (tables_ready) return;
    for (int u = 0; u < 8; u++) {
        alpha[u] = (u == 0) ? 1.0 / std::sqrt(2.0) : 1.0;
        for (int x = 0; x < 8; x++) {
            basis[u][x] = std::cos((2 * x + 1) * u * kPi / 16.0);
        }
    }
    tables_ready = true;
}

void dct1d(const double in[8], double out[8]) {
    for (int u = 0; u < 8; u++) {
        double sum = 0.0;
        for (int x = 0; x < 8; x++) {
            sum += in[x] * basis[u][x];
        }
        out[u] = 0.5 * alpha[u] * sum;
    }
}

void idct1d(const double in[8], double out[8]) {
    for (int x = 0; x < 8; x++) {
        double sum = 0.0;
        for (int u = 0; u < 8; u++) {
            sum += alpha[u] * in[u] * basis[u][x];
        }
        out[x] = 0.5 * sum;
    }
}

}  // namespace

void dct_8x8(const double block[8][8], double out[8][8]) {
    build_tables();
    double temp[8][8];
    for (int y = 0; y < 8; y++) {
        dct1d(block[y], temp[y]);
    }
    for (int x = 0; x < 8; x++) {
        double col_in[8], col_out[8];
        for (int y = 0; y < 8; y++) col_in[y] = temp[y][x];
        dct1d(col_in, col_out);
        for (int y = 0; y < 8; y++) out[y][x] = col_out[y];
    }
}

void idct_8x8(const double block[8][8], double out[8][8]) {
    build_tables();
    double temp[8][8];
    for (int x = 0; x < 8; x++) {
        double col_in[8], col_out[8];
        for (int y = 0; y < 8; y++) col_in[y] = block[y][x];
        idct1d(col_in, col_out);
        for (int y = 0; y < 8; y++) temp[y][x] = col_out[y];
    }
    for (int y = 0; y < 8; y++) {
        idct1d(temp[y], out[y]);
    }
}
