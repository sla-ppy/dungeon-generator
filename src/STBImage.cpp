#include "STBImage.h"

#include <algorithm>
#include <fmt/core.h>
#include <stb_image_resize.h>
#include <stb_image_write.h>
#include <stdexcept>

STBImage::STBImage(const std::string& path, size_t channels)
    : data { stbi_load(path.data(), &w, &h, &c, channels) } {
    if (!data) {
        throw std::runtime_error(fmt::format("stbi_load({}, ...) failed", path));
    }
}

void STBImage::free_data() {
    if (data) {
        if (manually_allocated) {
            delete[] data;
        } else {
            stbi_image_free(reinterpret_cast<void*>(data));
        }
    }
}

STBImage::STBImage(int w, int h, int c)
    : w(w)
    , h(h)
    , c(c)
    , data(new uint8_t[w * h * c])
    , manually_allocated(true) {
    // zero out
    std::fill_n(data, w * h * c, 0);
}

STBImage::~STBImage() noexcept {
    free_data();
}

STBImage::STBImage(STBImage&& o) noexcept
    : w(o.w)
    , h(o.h)
    , c(o.c)
    , data(o.data)
    , manually_allocated(o.manually_allocated) {
    // clear other's data to ensure no double-free or
    // use-after-free bugs
    o.data = nullptr;
}

STBImage& STBImage::operator=(STBImage&& o) noexcept {
    free_data();
    w = o.w;
    h = o.h;
    c = o.c;
    data = o.data;
    manually_allocated = o.manually_allocated;
    o.data = nullptr;
    return *this;
}

STBImage STBImage::resized(int new_w, int new_h) const {
    STBImage img(new_w, new_h, c);
    /*auto ret = stbir_resize_uint8_generic(data, w, h, 0, img.data, new_w, new_h, 0, c,
        STBIR_ALPHA_CHANNEL_NONE, 0, STBIR_EDGE_CLAMP, STBIR_FILTER_BOX, STBIR_COLORSPACE_LINEAR, nullptr);*/
    auto ret = stbir_resize_uint8(data, w, h, 0, img.data, new_w, new_h, 0, c);
    if (ret == 0) {
        throw std::runtime_error(fmt::format("resizing image from {}x{} to {}x{} failed", w, h, img.w, img.h));
    }
    return img;
}

void STBImage::copy_from(STBImage& from, int to_x, int to_y) {
    for (int x = 0; x < from.w; ++x) {
        for (int y = 0; y < from.h; ++y) {
            for (int ci = 0; ci < c; ++ci) {
                at(to_x + x, to_y + y, ci) = from.at(x, y, ci);
            }
        }
    }
}

uint8_t& STBImage::at(int x, int y, int c_i) {
    return data[x * h * c + y * c + c_i];
}

uint8_t STBImage::at(int x, int y, int c_i) const {
    return data[x * h * c + y * c + c_i];
}

void STBImage::write_to_file_png(const std::string& filename) {
    const std::string full_name = filename + ".png";
    const auto ret = stbi_write_png(full_name.c_str(), w, h, c, data, 0);
    if (ret == 0) {
        throw std::runtime_error(fmt::format("failed to write image to '{}.png'", filename));
    }
}
