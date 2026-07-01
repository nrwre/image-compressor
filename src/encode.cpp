#include "encode.h"

#include <fstream>
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

std::vector<uint8_t> extract_luma(const Bitmap& bmp, int padded_w, int padded_h) {
    std::vector<uint8_t> luma(static_cast<size_t>(padded_w) * padded_h);
    for (int row = 0; row < padded_h; row++) {
        int src_row = row < bmp.height ? row : bmp.height - 1;
        for (int col = 0; col < padded_w; col++) {
            int src_col = col < bmp.width ? col : bmp.width - 1;
            uint8_t r = bmp.at(src_col, src_row, 0);
            uint8_t g = bmp.at(src_col, src_row, 1);
            uint8_t b = bmp.at(src_col, src_row, 2);
            double lum = 0.299 * r + 0.587 * g + 0.114 * b;
            luma[row * padded_w + col] = static_cast<uint8_t>(lum + 0.5);
        }
    }
    return luma;
}

}  // namespace

void compress_bmp(const std::string& in_path, const std::string& out_path, int quality) {
    Bitmap bmp = load_bmp(in_path);

    int padded_w = pad_to_8(bmp.width);
    int padded_h = pad_to_8(bmp.height);
    std::vector<uint8_t> luma = extract_luma(bmp, padded_w, padded_h);

    int qtable[8][8];
    build_quant_table(quality, qtable);

    std::vector<uint8_t> stream;
    int prev_dc = 0;

    for (int by = 0; by < padded_h; by += 8) {
        for (int bx = 0; bx < padded_w; bx += 8) {
            double block[8][8];
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    block[y][x] = static_cast<double>(luma[(by + y) * padded_w + (bx + x)]) - 128.0;
                }
            }

            double freq[8][8];
            dct_8x8(block, freq);

            int quant[8][8];
            quantize_block(freq, qtable, quant);

            int zz[64];
            block_to_zigzag(quant, zz);

            encode_block(zz, prev_dc, stream);
        }
    }

    FileHeader header;
    header.magic = kFormatMagic;
    header.width = static_cast<uint32_t>(bmp.width);
    header.height = static_cast<uint32_t>(bmp.height);
    header.flags = pack_flags(quality, 0);

    std::ofstream out(out_path, std::ios::binary);
    if (!out) {
        throw std::runtime_error("could not write " + out_path);
    }
    out.write(reinterpret_cast<char*>(&header), sizeof(header));
    out.write(reinterpret_cast<char*>(stream.data()), static_cast<std::streamsize>(stream.size()));
}
