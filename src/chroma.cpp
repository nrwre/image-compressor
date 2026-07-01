#include "chroma.h"

std::vector<double> subsample_420(const std::vector<double>& plane, int width, int height) {
    int half_w = width / 2;
    int half_h = height / 2;
    std::vector<double> out(static_cast<size_t>(half_w) * half_h);

    for (int y = 0; y < half_h; y++) {
        for (int x = 0; x < half_w; x++) {
            double sum = plane[(2 * y) * width + (2 * x)] +
                         plane[(2 * y) * width + (2 * x + 1)] +
                         plane[(2 * y + 1) * width + (2 * x)] +
                         plane[(2 * y + 1) * width + (2 * x + 1)];
            out[y * half_w + x] = sum / 4.0;
        }
    }
    return out;
}

std::vector<double> upsample_420(const std::vector<double>& half_plane, int half_width, int half_height) {
    int width = half_width * 2;
    int height = half_height * 2;
    std::vector<double> out(static_cast<size_t>(width) * height);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            out[y * width + x] = half_plane[(y / 2) * half_width + (x / 2)];
        }
    }
    return out;
}
