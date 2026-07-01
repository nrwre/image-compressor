#pragma once

#include <cstdint>

struct YCbCr {
    double y;
    double cb;
    double cr;
};

YCbCr rgb_to_ycbcr(uint8_t r, uint8_t g, uint8_t b);
void ycbcr_to_rgb(double y, double cb, double cr, uint8_t& r, uint8_t& g, uint8_t& b);
