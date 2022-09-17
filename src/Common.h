#pragma once

#include <array>
#include <cstddef>
#include <string>

/**
 * A 2D fixed-size array.
 */
template<typename T, size_t Width, size_t Height>
class Array2D : public std::array<std::array<T, Width>, Height> {
public:
    using value_type = T;

    void fill(const value_type& value) {
        for (auto& row : *this) {
            row.fill(value);
        }
    }
    constexpr size_t width() const { return Width; }
    constexpr size_t height() const { return Height; }
};

enum class Tile {
    None, // wall, not generated, etc.
    Room,
    Corridor,
    Door,
    NextToRoom,
};

using Grid2D = Array2D<Tile, 20, 20>;

struct Error {
    bool is_err = false;
    std::string msg = "success";

    operator bool() const { return is_err; }

    Error() = default;
    Error(const std::string& msg)
        : is_err(true)
        , msg(msg) {
    }
};
