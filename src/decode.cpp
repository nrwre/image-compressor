#include "decode.h"

#include <fstream>
#include <iterator>
#include <stdexcept>
#include <vector>

#include "bitmap.h"
#include "chroma.h"
#include "color.h"
#include "dct.h"
#include "format.h"
#include "quant.h"
#include "rle.h"
#include "zigzag.h"

namespace {

int pad_to(int v, int n) {
    return ((v + n - 1) / n) * n;
}

std::vector<double> decode_plane(const std::vector<uint8_t>& stream, size_t& pos, int w, int h,
                                  const int qtable[8][8]) {
    std::vector<double> plane(static_cast<size_t>(w) * h);
    int prev_dc = 0;

    for (int by = 0; by < h; by += 8) {
        for (int bx = 0; bx < w; bx += 8) {
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
                    plane[(by + y) * w + (bx + x)] = block[y][x] + 128.0;
                }
            }
        }
    }
    return plane;
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
    int subsampling_mode = flags_subsampling(header.flags);

    int width = static_cast<int>(header.width);
    int height = static_cast<int>(header.height);

    int luma_qtable[8][8];
    build_quant_table(quality, luma_qtable);

    Bitmap bmp;
    bmp.width = width;
    bmp.height = height;
    bmp.pixels.resize(static_cast<size_t>(width) * height * 3);

    if (subsampling_mode == 0) {
        int padded_w = pad_to(width, 8);
        int padded_h = pad_to(height, 8);

        size_t pos = 0;
        std::vector<double> y_plane = decode_plane(stream, pos, padded_w, padded_h, luma_qtable);

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                double v = y_plane[y * padded_w + x];
                v = v < 0 ? 0 : (v > 255 ? 255 : v);
                uint8_t val = static_cast<uint8_t>(v + 0.5);
                bmp.at(x, y, 0) = val;
                bmp.at(x, y, 1) = val;
                bmp.at(x, y, 2) = val;
            }
        }
    } else {
        int chroma_qtable[8][8];
        build_chroma_quant_table(quality, chroma_qtable);

        int padded_w = pad_to(width, 16);
        int padded_h = pad_to(height, 16);

        size_t pos = 0;
        std::vector<double> y_plane = decode_plane(stream, pos, padded_w, padded_h, luma_qtable);
        std::vector<double> cb_sub = decode_plane(stream, pos, padded_w / 2, padded_h / 2, chroma_qtable);
        std::vector<double> cr_sub = decode_plane(stream, pos, padded_w / 2, padded_h / 2, chroma_qtable);

        std::vector<double> cb_plane = upsample_420(cb_sub, padded_w / 2, padded_h / 2);
        std::vector<double> cr_plane = upsample_420(cr_sub, padded_w / 2, padded_h / 2);

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                double yy = y_plane[y * padded_w + x];
                double cb = cb_plane[y * padded_w + x];
                double cr = cr_plane[y * padded_w + x];
                uint8_t r, g, b;
                ycbcr_to_rgb(yy, cb, cr, r, g, b);
                bmp.at(x, y, 0) = r;
                bmp.at(x, y, 1) = g;
                bmp.at(x, y, 2) = b;
            }
        }
    }

    save_bmp(out_path, bmp);
}
