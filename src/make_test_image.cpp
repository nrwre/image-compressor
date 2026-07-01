#include <cmath>
#include <cstdlib>
#include <iostream>

#include "bitmap.h"

int main(int argc, char** argv) {
    int size = 256;
    std::string out_path = "test.bmp";
    if (argc > 1) out_path = argv[1];
    if (argc > 2) size = std::atoi(argv[2]);

    Bitmap bmp;
    bmp.width = size;
    bmp.height = size;
    bmp.pixels.resize(static_cast<size_t>(size) * size * 3);

    unsigned int seed = 12345;
    auto next_rand = [&seed]() {
        seed = seed * 1103515245 + 12345;
        return (seed >> 16) & 0x7FFF;
    };

    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            int r = (x * 255) / size;
            int g = (y * 255) / size;
            int b = 128 + static_cast<int>(60.0 * std::sin(x * 0.05) * std::cos(y * 0.05));

            bool in_circle = (x - size / 3) * (x - size / 3) + (y - size / 3) * (y - size / 3) < (size / 6) * (size / 6);
            if (in_circle) {
                r = 220;
                g = 40;
                b = 40;
            }

            bool in_box = x > size * 2 / 3 && x < size * 2 / 3 + size / 5 && y > size / 2 && y < size / 2 + size / 5;
            if (in_box) {
                r = 30;
                g = 200;
                b = 90;
            }

            int noise = static_cast<int>(next_rand() % 12) - 6;
            r += noise;
            g += noise;
            b += noise;

            r = r < 0 ? 0 : (r > 255 ? 255 : r);
            g = g < 0 ? 0 : (g > 255 ? 255 : g);
            b = b < 0 ? 0 : (b > 255 ? 255 : b);

            bmp.at(x, y, 0) = static_cast<uint8_t>(r);
            bmp.at(x, y, 1) = static_cast<uint8_t>(g);
            bmp.at(x, y, 2) = static_cast<uint8_t>(b);
        }
    }

    save_bmp(out_path, bmp);
    std::cout << "wrote " << out_path << " (" << size << "x" << size << ")\n";
    return 0;
}
