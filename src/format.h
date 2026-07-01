#pragma once

#include <cstdint>

struct FileHeader {
    uint32_t magic;
    uint32_t width;
    uint32_t height;
    uint32_t flags;
};

static_assert(sizeof(FileHeader) == 16, "FileHeader must be 16 bytes");

constexpr uint32_t kFormatMagic = 'I' | ('C' << 8) | ('C' << 16) | ('1' << 24);

inline uint32_t pack_flags(int quality, int subsampling_mode) {
    return (static_cast<uint32_t>(quality) & 0xFF) | ((static_cast<uint32_t>(subsampling_mode) & 0xFF) << 8);
}

inline int flags_quality(uint32_t flags) {
    return static_cast<int>(flags & 0xFF);
}

inline int flags_subsampling(uint32_t flags) {
    return static_cast<int>((flags >> 8) & 0xFF);
}
