#pragma once

#include <cstdint>
#include <vector>

void encode_block(const int zz[64], int& prev_dc, std::vector<uint8_t>& out);
void decode_block(const std::vector<uint8_t>& buf, size_t& pos, int& prev_dc, int zz[64]);
