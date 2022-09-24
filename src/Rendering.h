#pragma once

#include "Common.h"

Error render(const Grid2D& grid, const std::string& filename, size_t scale = 1, bool use_textures = true);
