#pragma once

#include <cstddef>
#include <stb_image.h>
#include <string_view>

/**
 * @brief RAII wrapper around the stbi_* api.
 * This ensures that memory is free'd appropriately.
 *
 * stb_image is a C library, so it has no RAII builtin, which
 * makes using it a massive pain. With this wrapper, the
 * lifetime of this class results in the
 */
struct STBImage {
    /**
     * @brief Loads the image into memory, together with size and required number of
     * channels.
     * @param path path to image
     * @param channels number of channels (examples: 3=RGB, 4=RGBA)
     */
    STBImage(const std::string& path, size_t channels);
    /**
     * @brief Creates an image of `w x h` pixels with `c` channels.
     * @param w width
     * @param h height
     * @param c number of channels (e.g. 3 for RGB)
     */
    STBImage(int w, int h, int c);
    /**
     * @brief Frees the image memory
     */
    ~STBImage() noexcept;
    // disallow copying, since that would copy the pointer, and after one
    // instance frees it, its invalid
    STBImage(const STBImage&) = delete;
    // ensure moved-from object doesn't hold data
    STBImage(STBImage&& o) noexcept;

    /**
     * @brief Returns a copy of the image, resized to the desired `w x h`.
     * Uses linear colorspace, and a box filter. Assumes *no alpha channel*.
     * @param new_w desired new width of the image
     * @param new_h desired new height of the image
     * @return new image, with its own allocated memory
     */
    STBImage resized(int new_w, int new_h) const;

    void copy_from(STBImage& from, int to_x, int to_y);

    uint8_t& at(int x, int y, int c_i);
    uint8_t at(int x, int y, int c_i) const;

    void write_to_file_png(const std::string& filename);

    // width
    int w { 0 };
    // height
    int h { 0 };
    // channels
    int c { 0 };
    // image data
    uint8_t* data { nullptr };

private:
    STBImage() = default;

    // decides whether to use delete[] or stb_image_free
    // in the destructor (DO NOT USE UNLESS IN resize())
    bool manually_allocated { false };
};
