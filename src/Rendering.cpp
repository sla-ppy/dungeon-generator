#include "STBImage.h"
#include <boost/process.hpp>
#include <boost/process/detail/child_decl.hpp>
#include <boost/process/spawn.hpp>
#include <chrono>
#include <filesystem>
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
    case Tile::NextToRoom:
        return { 255, 165, 0 };
    default:
        l::error("unhandled tile type in get_color_for_tile: {}", int(tile));
        return { 255, 0, 0 };
    }
}

std::vector<std::string> collect_file_names(const std::string& path, const std::string& extension = ".png") {
    std::vector<std::string> filenames;
    for (auto& p : std::filesystem::directory_iterator(path)) {
        if (p.path().extension() == extension) {
            filenames.push_back(p.path().filename());
            std::string loaded_file = p.path().filename();
        }
    }
    return filenames;
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
    STBImage img(grid.width(), grid.height(), 3);

    // maps each tile on the grid to a color, writes that color into the array
    for (size_t y = 0; y < grid.height(); ++y) {
        for (size_t x = 0; x < grid.width(); ++x) {
            const auto color = get_color_for_tile(grid[x][y]);
            // iterate through all 3 color channels: r, g, b.
            for (size_t c = 0; c < 3; ++c) {
                img.at(x, y, c) = color[c];
            }
        }
    }

    const std::string& path { "./assets/tiles/" };
    std::vector<std::string> file_names = collect_file_names(path, ".png");

    const int channels { 3 }; // RGB
    std::vector<STBImage> textures;
    for (const auto& file_name : file_names) {
        textures.push_back(STBImage(path + file_name, channels));
    }

    // if scale is other than 1, rescale and render into file.
    // otherwise, simply render it into the file.
    if (scale != 1) {
        STBImage scaled = img.resized(grid.width() * scale, grid.height() * scale);
        l::info("resized input from {}x{} to {}x{} ({}x)", grid.width(), grid.height(), scaled.w, scaled.h, scale);
        l::info("writing scaled image to '{}.png'", filename);
        scaled.write_to_file_png(filename);
    } else {
        l::info("writing image to '{}.png'", filename);
        img.write_to_file_png(filename);
    }

    l::info("opening image viewer", filename);

    auto child = boost::process::child(fmt::format("xdg-open {}.png", filename),
        boost::process::std_in.close(),
        boost::process::std_out > boost::process::null,
        boost::process::std_err > boost::process::null);
    child.join();

    return {};
}
