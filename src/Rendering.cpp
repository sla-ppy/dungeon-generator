#include <chrono>
#include <fmt/core.h>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include "Log.h"
#include "Rendering.h"

using Color = std::array<uint8_t, 3>;

Color get_color_for_tile(Tile tile) {
    switch (tile) {
    case Tile::None:
        return { 0, 0, 0 };
    case Tile::Room:
        return { 64, 64, 255 };
    case Tile::Corridor:
        return { 128, 128, 128 };
    case Tile::Door:
        return { 0, 255, 0 };
    }
}

Error render(const Grid2D& grid, const std::string& filename) {

    std::vector<uint8_t> img;
    img.resize(grid.width() * grid.height() * 3);
    std::fill(img.begin(), img.end(), 128);

    for (size_t y = 0; y < grid.height(); ++y) {
        for (size_t x = 0; x < grid.width(); ++x) {
            const auto color = get_color_for_tile(grid[x][y]);
            for (size_t c = 0; c < 3; ++c) {
                img.at(x * grid.height() * 3 + y * 3 + c) = color[c];
            }
        }
    }

    auto ret = stbi_write_png(fmt::format("{}.png", filename).c_str(), grid.width(), grid.height(), 3, img.data(), 0);
    if (ret == 0) {
        return { "failed to write image" };
    }
    std::system(fmt::format("xdg-open {}.png", filename).c_str());

    return {};
}