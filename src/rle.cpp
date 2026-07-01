#include "rle.h"

namespace {

const uint8_t kEobMarker = 0xFF;

void write_varint(std::vector<uint8_t>& out, int32_t value) {
    uint32_t zz = (static_cast<uint32_t>(value) << 1) ^ static_cast<uint32_t>(value >> 31);
    while (zz >= 0x80) {
        out.push_back(static_cast<uint8_t>(zz) | 0x80);
        zz >>= 7;
    }
    out.push_back(static_cast<uint8_t>(zz));
}

int32_t read_varint(const std::vector<uint8_t>& buf, size_t& pos) {
    uint32_t zz = 0;
    int shift = 0;
    while (true) {
        uint8_t b = buf[pos++];
        zz |= static_cast<uint32_t>(b & 0x7F) << shift;
        if (!(b & 0x80)) break;
        shift += 7;
    }
    return static_cast<int32_t>(zz >> 1) ^ -static_cast<int32_t>(zz & 1);
}

}  // namespace

void encode_block(const int zz[64], int& prev_dc, std::vector<uint8_t>& out) {
    write_varint(out, zz[0] - prev_dc);
    prev_dc = zz[0];

    int run = 0;
    for (int k = 1; k < 64; k++) {
        if (zz[k] == 0) {
            run++;
            continue;
        }
        while (run > 62) {
            out.push_back(62);
            write_varint(out, 0);
            run -= 62;
        }
        out.push_back(static_cast<uint8_t>(run));
        write_varint(out, zz[k]);
        run = 0;
    }
    out.push_back(kEobMarker);
}

void decode_block(const std::vector<uint8_t>& buf, size_t& pos, int& prev_dc, int zz[64]) {
    int dc_delta = read_varint(buf, pos);
    prev_dc += dc_delta;
    zz[0] = prev_dc;

    for (int k = 1; k < 64; k++) zz[k] = 0;

    int k = 1;
    while (true) {
        uint8_t marker = buf[pos++];
        if (marker == kEobMarker) break;
        int run = marker;
        int32_t value = read_varint(buf, pos);
        k += run;
        zz[k] = value;
        k++;
    }
}
