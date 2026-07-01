#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct Bitmap {
    int width = 0;
    int height = 0;
    std::vector<uint8_t> pixels;

    uint8_t& at(int x, int y, int channel) {
        return pixels[(y * width + x) * 3 + channel];
    }

    uint8_t at(int x, int y, int channel) const {
        return pixels[(y * width + x) * 3 + channel];
    }
};

Bitmap load_bmp(const std::string& path);
void save_bmp(const std::string& path, const Bitmap& bmp);
