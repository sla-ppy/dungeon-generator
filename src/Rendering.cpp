#include <boost/process.hpp>
#include <boost/process/detail/child_decl.hpp>
#include <boost/process/spawn.hpp>
#include <chrono>
#include <fmt/core.h>
#include <vector>

// set filter to box filter, which is sharp when integer-scaling
// images later.
#define STBIR_DEFAULT_FILTER_UPSAMPLE STBIR_FILTER_BOX

// stb_image has some weird warnings here, we ignore them purposefully
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>
#pragma GCC diagnostic pop

#include "Log.h"
#include "Rendering.h"

/**
 * An 8-bit RGB color type. Order: [r, g, b].
 */
using Color = std::array<uint8_t, 3>;

/**
 * @brief Returns a color for the specified tile type, or
 * red for tile types which are unknown (this should never happen).
 */
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

/**
 * @brief Renders the grid into a PNG file.
 * @param grid Grid to render.
 * @param filename Filename or path with filename to write to, without extension.
 * @param scale Scale to scale the image to (at least 1). With a scale of 2, for example,
 * each grid pixel becomes a 2x2 pixel area in the image.
 * @return An error if anything went wrong, explaining the issue in the message field.
 */
Error render(const Grid2D& grid, const std::string& filename, size_t scale) {
    if (scale < 1) {
        l::error("render scale must be >= 1, got {}", scale);
        return { "invalid render scale" };
    }

    // an array of w*h RGB values, thus (w * h) * 3(bytes)
    std::vector<uint8_t> img;
    img.resize(grid.width() * grid.height() * 3);

    // maps each tile on the grid to a color, writes that color into the array
    for (size_t y = 0; y < grid.height(); ++y) {
        for (size_t x = 0; x < grid.width(); ++x) {
            const auto color = get_color_for_tile(grid[x][y]);
            // iterate through all 3 color channels: r, g, b.
            for (size_t c = 0; c < 3; ++c) {
                img.at(x * grid.height() * 3 + y * 3 + c) = color[c];
            }
        }
    }

    // if scale is other than 1, rescale and render into file.
    // otherwise, simply render it into the file.
    if (scale != 1) {
        std::vector<uint8_t> scaled_img;
        const size_t scaled_w = grid.width() * scale;
        const size_t scaled_h = grid.height() * scale;
        l::info("resizing input from {}x{} to {}x{} ({}x)", grid.width(), grid.height(), scaled_w, scaled_h, scale);
        // each pixel takes `scale * scale` memory now
        scaled_img.resize(scaled_w * scaled_h * 3);
        // resize using stb_image, the filter is set in the top of the file
        auto ret = stbir_resize_uint8(img.data(), grid.width(), grid.height(), 0,
            scaled_img.data(), scaled_w, scaled_h, 0, 3);
        if (ret == 0) {
            return { fmt::format("failed to resize image with factor {}", scale) };
        }
        l::info("writing image to '{}.png'", filename);
        ret = stbi_write_png(fmt::format("{}.png", filename).c_str(), scaled_w, scaled_h, 3, scaled_img.data(), 0);
        if (ret == 0) {
            return { "failed to write image" };
        }
    } else {
        l::info("writing image to '{}.png'", filename);
        auto ret = stbi_write_png(fmt::format("{}.png", filename).c_str(), grid.width(), grid.height(), 3, img.data(), 0);
        if (ret == 0) {
            return { "failed to write image" };
        }
    }
    std::system(fmt::format("xdg-open {}.png", filename).c_str());

    return {};
}
