#pragma once

inline const int kZigzagOrder[64] = {
    0,  1,  8,  16, 9,  2,  3,  10,
    17, 24, 32, 25, 18, 11, 4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13, 6,  7,  14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63};

inline void block_to_zigzag(const int block[8][8], int out[64]) {
    for (int k = 0; k < 64; k++) {
        int idx = kZigzagOrder[k];
        out[k] = block[idx / 8][idx % 8];
    }
}

inline void zigzag_to_block(const int zz[64], int out[8][8]) {
    for (int k = 0; k < 64; k++) {
        int idx = kZigzagOrder[k];
        out[idx / 8][idx % 8] = zz[k];
    }
}
