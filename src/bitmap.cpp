#include "bitmap.h"

#include <fstream>
#include <stdexcept>

namespace {

void write_u16(std::ofstream& out, uint16_t v) {
    uint8_t b[2] = {static_cast<uint8_t>(v & 0xFF), static_cast<uint8_t>((v >> 8) & 0xFF)};
    out.write(reinterpret_cast<char*>(b), 2);
}

void write_u32(std::ofstream& out, uint32_t v) {
    uint8_t b[4] = {
        static_cast<uint8_t>(v & 0xFF),
        static_cast<uint8_t>((v >> 8) & 0xFF),
        static_cast<uint8_t>((v >> 16) & 0xFF),
        static_cast<uint8_t>((v >> 24) & 0xFF)};
    out.write(reinterpret_cast<char*>(b), 4);
}

uint16_t read_u16(std::ifstream& in) {
    uint8_t b[2];
    in.read(reinterpret_cast<char*>(b), 2);
    return static_cast<uint16_t>(b[0] | (b[1] << 8));
}

uint32_t read_u32(std::ifstream& in) {
    uint8_t b[4];
    in.read(reinterpret_cast<char*>(b), 4);
    return static_cast<uint32_t>(b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24));
}

int row_padded_bytes(int width) {
    return ((width * 3 + 3) / 4) * 4;
}

}  // namespace

Bitmap load_bmp(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw std::runtime_error("could not open " + path);
    }

    char sig[2];
    in.read(sig, 2);
    if (sig[0] != 'B' || sig[1] != 'M') {
        throw std::runtime_error(path + " is not a BMP file");
    }

    read_u32(in);
    read_u32(in);
    uint32_t pixel_offset = read_u32(in);

    read_u32(in);
    int32_t width_raw = static_cast<int32_t>(read_u32(in));
    int32_t height_raw = static_cast<int32_t>(read_u32(in));
    read_u16(in);
    uint16_t bpp = read_u16(in);
    uint32_t compression = read_u32(in);

    if (bpp != 24 || compression != 0) {
        throw std::runtime_error(path + " must be an uncompressed 24-bit BMP");
    }

    int width = width_raw;
    int height = height_raw < 0 ? -height_raw : height_raw;
    bool bottom_up = height_raw > 0;

    Bitmap bmp;
    bmp.width = width;
    bmp.height = height;
    bmp.pixels.resize(static_cast<size_t>(width) * height * 3);

    in.seekg(pixel_offset, std::ios::beg);

    int padded = row_padded_bytes(width);
    std::vector<uint8_t> row(padded);

    for (int y = 0; y < height; y++) {
        in.read(reinterpret_cast<char*>(row.data()), padded);
        int dest_row = bottom_up ? (height - 1 - y) : y;
        for (int x = 0; x < width; x++) {
            uint8_t b = row[x * 3 + 0];
            uint8_t g = row[x * 3 + 1];
            uint8_t r = row[x * 3 + 2];
            bmp.at(x, dest_row, 0) = r;
            bmp.at(x, dest_row, 1) = g;
            bmp.at(x, dest_row, 2) = b;
        }
    }

    return bmp;
}

void save_bmp(const std::string& path, const Bitmap& bmp) {
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        throw std::runtime_error("could not write " + path);
    }

    int padded = row_padded_bytes(bmp.width);
    uint32_t pixel_data_size = static_cast<uint32_t>(padded) * bmp.height;
    uint32_t file_size = 54 + pixel_data_size;

    out.put('B');
    out.put('M');
    write_u32(out, file_size);
    write_u32(out, 0);
    write_u32(out, 54);

    write_u32(out, 40);
    write_u32(out, static_cast<uint32_t>(bmp.width));
    write_u32(out, static_cast<uint32_t>(bmp.height));
    write_u16(out, 1);
    write_u16(out, 24);
    write_u32(out, 0);
    write_u32(out, pixel_data_size);
    write_u32(out, 2835);
    write_u32(out, 2835);
    write_u32(out, 0);
    write_u32(out, 0);

    std::vector<uint8_t> row(padded, 0);
    for (int y = bmp.height - 1; y >= 0; y--) {
        for (int x = 0; x < bmp.width; x++) {
            row[x * 3 + 0] = bmp.at(x, y, 2);
            row[x * 3 + 1] = bmp.at(x, y, 1);
            row[x * 3 + 2] = bmp.at(x, y, 0);
        }
        out.write(reinterpret_cast<char*>(row.data()), padded);
    }
}
