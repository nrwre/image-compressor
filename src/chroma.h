#pragma once

#include <vector>

std::vector<double> subsample_420(const std::vector<double>& plane, int width, int height);
std::vector<double> upsample_420(const std::vector<double>& half_plane, int half_width, int half_height);
