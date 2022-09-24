#include "Common.h"
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

#define CHANNELS 4

/**
 * An 8-bit RGB color type.
 */
using Color = std::array<uint8_t, CHANNELS>;

/**
 * @brief Returns a color for the specified tile type, or
 * red for tile types which are unknown (this should never happen).
 */
Color color_for_tile(Tile tile) {
    switch (tile) {
    case Tile::None:
        return { 0, 0, 0, 255 };
    case Tile::Room:
        return { 64, 64, 255, 255 };
    case Tile::Corridor:
        return { 128, 128, 128, 255 };
    case Tile::Door:
        return { 0, 255, 0, 255 };
    case Tile::NextToRoom:
        return { 255, 165, 0, 255 };
    case Tile::Corner:
        return { 128, 0, 128, 255 };
    default:
        l::error("unhandled tile type in get_color_for_tile: {}", int(tile));
        return { 255, 0, 0, 255 };
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
 * @brief Returns a stringified texture name of a given tile.
 */
std::string texture_name_for_tile(Tile tile) {
    switch (tile) {
    case Tile::None:
        return "none";
    case Tile::Room:
        return "none";
    case Tile::Corridor:
        return "none";
    case Tile::Door:
        return "door";
    case Tile::NextToRoom:
        return "wall";
    case Tile::Corner:
        return "wall";
    default:
        l::error("unhandled tile type in texture_name_for_tile: {}", int(tile));
        return "none";
    }
}

/**
 * @brief Loads textures from a vector of filenames.
 * @param base_path base path for each file, prepended
 * @param filenames filenames to load
 * @return map of file name *stems* to images
 */
std::unordered_map<std::string, STBImage> load_textures(const std::string& base_path, const std::vector<std::string>& filenames) {
    std::unordered_map<std::string, STBImage> images;
    for (const auto& filename : filenames) {
        const auto filename_stem = std::filesystem::path(filename).stem().string();
        images.emplace(filename_stem, STBImage(base_path + filename, CHANNELS));
    }
    return images;
}

/**
 * @brief Fills the given image according to the tile types in the grid.
 * Grid and image have to be the same size. Assuming CHANNELS color channels.
 * @param image image to fill
 * @param grid grid to take tile info from
 * @return error if something went wrong
 */
Error fill_image(STBImage& image, const Grid2D& grid) {
    if (size_t(image.w) != grid.width()) {
        return { "image width != grid width" };
    }
    if (size_t(image.h) != grid.height()) {
        return { "image witdh != grid height" };
    }
    // maps each tile on the grid to a color, writes that color into the array
    for (size_t y = 0; y < grid.height(); ++y) {
        for (size_t x = 0; x < grid.width(); ++x) {
            const auto color = color_for_tile(grid[x][y]);
            // iterate through all CHANNELS color channels: r, g, b.
            for (size_t c = 0; c < CHANNELS; ++c) {
                image.at(x, y, c) = color[c];
            }
        }
    }
    return {};
}

/**
 * @brief Renders the grid into a PNG file.
 * @param grid Grid to render.
 * @param filename Filename or path with filename to write to, without extension.
 * @param scale Scale to scale the image to (at least 1). With a scale of 2, for example,
 * each grid pixel becomes a 2x2 pixel area in the image.
 * @return An error if anything went wrong, explaining the issue in the message field.
 */
Error render(const Grid2D& grid, const std::string& filename, size_t scale, bool use_textures) {
    if (scale < 1) {
        l::error("render scale must be >= 1, got {}", scale);
        return { "invalid render scale" };
    }

    if (!use_textures) {
        // an array of w*h RGB values, thus (w * h) * CHANNELS(bytes)
        STBImage img(grid.width(), grid.height(), CHANNELS);

        auto error = fill_image(img, grid);
        if (error) {
            return { fmt::format("failed to fill image from grid: {}", error.msg) };
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
    } else {
        const std::string& path { "./assets/tiles/" };
        std::vector<std::string> filenames = collect_file_names(path, ".png");
        std::unordered_map<std::string, STBImage> textures = load_textures(path, filenames);

        // resize textures
        for (auto& name_texture_pair : textures) {
            name_texture_pair.second = name_texture_pair.second.resized(scale, scale);
        }

        // scale image to `scale`
        STBImage scaled(grid.width() * scale, grid.height() * scale, CHANNELS);

        for (size_t y = 0; y < grid.height(); ++y) {
            for (size_t x = 0; x < grid.width(); ++x) {
                const auto texture_name = texture_name_for_tile(grid[x][y]);
                if (textures.find(texture_name) != textures.end()) {
                    scaled.copy_from(textures.at(texture_name), x * scale, y * scale);
                } else {
                    l::error("no texture loaded for tile type {}", int(grid[x][y]));
                }
            }
        }

        scaled.write_to_file_png(filename);
    }

    l::info("opening image viewer", filename);

    spawn_process_silently(fmt::format("xdg-open {}.png", filename));

    return {};
}
