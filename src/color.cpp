#include "color.h"

namespace {

double clamp255(double v) {
    if (v < 0.0) return 0.0;
    if (v > 255.0) return 255.0;
    return v;
}

}  // namespace

YCbCr rgb_to_ycbcr(uint8_t r, uint8_t g, uint8_t b) {
    YCbCr out;
    out.y = 0.299 * r + 0.587 * g + 0.114 * b;
    out.cb = -0.168736 * r - 0.331264 * g + 0.5 * b + 128.0;
    out.cr = 0.5 * r - 0.418688 * g - 0.081312 * b + 128.0;
    return out;
}

void ycbcr_to_rgb(double y, double cb, double cr, uint8_t& r, uint8_t& g, uint8_t& b) {
    double rr = y + 1.402 * (cr - 128.0);
    double gg = y - 0.344136 * (cb - 128.0) - 0.714136 * (cr - 128.0);
    double bb = y + 1.772 * (cb - 128.0);

    r = static_cast<uint8_t>(clamp255(rr) + 0.5);
    g = static_cast<uint8_t>(clamp255(gg) + 0.5);
    b = static_cast<uint8_t>(clamp255(bb) + 0.5);
}
