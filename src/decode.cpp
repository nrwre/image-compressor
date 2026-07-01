#include "decode.h"

#include <fstream>
#include <iterator>
#include <stdexcept>
#include <vector>

#include "bitmap.h"
#include "dct.h"
#include "format.h"
#include "quant.h"
#include "rle.h"
#include "zigzag.h"

namespace {

int pad_to_8(int v) {
    return ((v + 7) / 8) * 8;
}

}  // namespace

void decompress_file(const std::string& in_path, const std::string& out_path) {
    std::ifstream in(in_path, std::ios::binary);
    if (!in) {
        throw std::runtime_error("could not open " + in_path);
    }

    FileHeader header;
    in.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (header.magic != kFormatMagic) {
        throw std::runtime_error(in_path + " is not a recognized .icc file");
    }

    std::vector<uint8_t> stream((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

    int quality = flags_quality(header.flags);
    int qtable[8][8];
    build_quant_table(quality, qtable);

    int width = static_cast<int>(header.width);
    int height = static_cast<int>(header.height);
    int padded_w = pad_to_8(width);
    int padded_h = pad_to_8(height);

    std::vector<uint8_t> luma(static_cast<size_t>(padded_w) * padded_h);

    size_t pos = 0;
    int prev_dc = 0;

    for (int by = 0; by < padded_h; by += 8) {
        for (int bx = 0; bx < padded_w; bx += 8) {
            int zz[64];
            decode_block(stream, pos, prev_dc, zz);

            int quant[8][8];
            zigzag_to_block(zz, quant);

            double freq[8][8];
            dequantize_block(quant, qtable, freq);

            double block[8][8];
            idct_8x8(freq, block);

            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    double v = block[y][x] + 128.0;
                    v = v < 0 ? 0 : (v > 255 ? 255 : v);
                    luma[(by + y) * padded_w + (bx + x)] = static_cast<uint8_t>(v + 0.5);
                }
            }
        }
    }

    Bitmap bmp;
    bmp.width = width;
    bmp.height = height;
    bmp.pixels.resize(static_cast<size_t>(width) * height * 3);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint8_t v = luma[y * padded_w + x];
            bmp.at(x, y, 0) = v;
            bmp.at(x, y, 1) = v;
            bmp.at(x, y, 2) = v;
        }
    }

    save_bmp(out_path, bmp);
}
