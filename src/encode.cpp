#include "encode.h"

#include <fstream>
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

void encode_plane(const std::vector<double>& plane, int w, int h, const int qtable[8][8],
                   std::vector<uint8_t>& stream) {
    int prev_dc = 0;
    for (int by = 0; by < h; by += 8) {
        for (int bx = 0; bx < w; bx += 8) {
            double block[8][8];
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    block[y][x] = plane[(by + y) * w + (bx + x)] - 128.0;
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
}

}  // namespace

void compress_bmp(const std::string& in_path, const std::string& out_path, int quality, bool color) {
    Bitmap bmp = load_bmp(in_path);

    int luma_qtable[8][8];
    build_quant_table(quality, luma_qtable);

    std::vector<uint8_t> stream;
    int subsampling_mode = color ? 1 : 0;

    if (!color) {
        int padded_w = pad_to(bmp.width, 8);
        int padded_h = pad_to(bmp.height, 8);

        std::vector<double> y_plane(static_cast<size_t>(padded_w) * padded_h);
        for (int row = 0; row < padded_h; row++) {
            int src_row = row < bmp.height ? row : bmp.height - 1;
            for (int col = 0; col < padded_w; col++) {
                int src_col = col < bmp.width ? col : bmp.width - 1;
                uint8_t r = bmp.at(src_col, src_row, 0);
                uint8_t g = bmp.at(src_col, src_row, 1);
                uint8_t b = bmp.at(src_col, src_row, 2);
                y_plane[row * padded_w + col] = 0.299 * r + 0.587 * g + 0.114 * b;
            }
        }

        encode_plane(y_plane, padded_w, padded_h, luma_qtable, stream);
    } else {
        int chroma_qtable[8][8];
        build_chroma_quant_table(quality, chroma_qtable);

        int padded_w = pad_to(bmp.width, 16);
        int padded_h = pad_to(bmp.height, 16);

        std::vector<double> y_plane(static_cast<size_t>(padded_w) * padded_h);
        std::vector<double> cb_plane(static_cast<size_t>(padded_w) * padded_h);
        std::vector<double> cr_plane(static_cast<size_t>(padded_w) * padded_h);

        for (int row = 0; row < padded_h; row++) {
            int src_row = row < bmp.height ? row : bmp.height - 1;
            for (int col = 0; col < padded_w; col++) {
                int src_col = col < bmp.width ? col : bmp.width - 1;
                uint8_t r = bmp.at(src_col, src_row, 0);
                uint8_t g = bmp.at(src_col, src_row, 1);
                uint8_t b = bmp.at(src_col, src_row, 2);
                YCbCr px = rgb_to_ycbcr(r, g, b);
                y_plane[row * padded_w + col] = px.y;
                cb_plane[row * padded_w + col] = px.cb;
                cr_plane[row * padded_w + col] = px.cr;
            }
        }

        std::vector<double> cb_sub = subsample_420(cb_plane, padded_w, padded_h);
        std::vector<double> cr_sub = subsample_420(cr_plane, padded_w, padded_h);

        encode_plane(y_plane, padded_w, padded_h, luma_qtable, stream);
        encode_plane(cb_sub, padded_w / 2, padded_h / 2, chroma_qtable, stream);
        encode_plane(cr_sub, padded_w / 2, padded_h / 2, chroma_qtable, stream);
    }

    FileHeader header;
    header.magic = kFormatMagic;
    header.width = static_cast<uint32_t>(bmp.width);
    header.height = static_cast<uint32_t>(bmp.height);
    header.flags = pack_flags(quality, subsampling_mode);

    std::ofstream out(out_path, std::ios::binary);
    if (!out) {
        throw std::runtime_error("could not write " + out_path);
    }
    out.write(reinterpret_cast<char*>(&header), sizeof(header));
    out.write(reinterpret_cast<char*>(stream.data()), static_cast<std::streamsize>(stream.size()));
}
