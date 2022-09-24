#include "Generation.h"
#include "Log.h"

#include <random>

namespace Random {
size_t generate(size_t min, size_t max) {
    static std::mt19937 mt { std::random_device {}() };
    std::uniform_int_distribution distribution { min, max };
    return distribution(mt);
}
}

/**
 * @brief Fills a rectangular area in the grid with a given tile.
 * @param grid grid to change
 * @param x start x of the rectangle
 * @param y start y of the rectangle
 * @param w width of the rectangle
 * @param h height of the rectangle
 * @param tile tile to fill the rectangle with
 */
void fill_area(Grid2D& grid, size_t x, size_t y, size_t w, size_t h, Tile tile) {
    for (size_t y_index = y; y_index < y + h; ++y_index) {
        for (size_t x_index = x; x_index < x + w; ++x_index) {
            grid[x_index][y_index] = tile;
        }
    }
}

/**
 * @brief Fills the corners of a rectangle on the grid with a specific tile.
 * @param grid grid to change
 * @param x start x
 * @param y start y
 * @param w width of the rectangle
 * @param h height of the rectangle
 * @param tile corner tile
 */
void fill_corners(Grid2D& grid, size_t x, size_t y, size_t w, size_t h, Tile tile) {
    // FIXME: the w and h might be swapped in some places
    for (size_t y_index = y; y_index < (y + h); ++y_index) {
        for (size_t x_index = x; x_index < (x + w); ++x_index) {
            if (x_index == x && y_index == y) {
                grid[x_index][y_index] = tile; // TOP-LEFT
            } else if (x_index == x && y_index == (y + h) - 1) {
                grid[x_index][y_index] = tile; // TOP-RIGHT
            } else if (x_index == (x + w) - 1 && y_index == y) {
                grid[x_index][y_index] = tile; // TOP-RIGHT
            } else if (x_index == (x + w) - 1 && y_index == (y + h) - 1) {
                grid[x_index][y_index] = tile; // BOTTOM-RIGHT
            }
        }
    }
}

void generate_random_tile(Grid2D& grid, size_t start_x, size_t start_y, size_t w, size_t h, Tile tile) {
    // BE CAREFUL that x + w and y + h don't exceed the bounds
    const auto x = Random::generate(start_x, start_x + w);
    const auto y = Random::generate(start_y, start_y + h);
    grid[x][y] = tile;
}

void generate_doors(Grid2D& grid, size_t start_x, size_t start_y, size_t w, size_t h, Tile tile) {
    size_t exact { 0 };
    size_t expected { Random::generate(1, 3) };

    /*
     * pick a wall to generate the guaranteed first door in
     *   3   1
     * 0 *---*
     *   |   |
     * 2 *---*
     */

    const size_t wall = Random::generate(0, 3);

    switch (wall) {
    case 0:
        generate_random_tile(grid, start_x + 1, start_y, w - 2, 0, tile);
        break;
    case 1:
        generate_random_tile(grid, start_x + w - 1, start_y + 1, 0, h - 2, tile);
        break;
    case 2:
        generate_random_tile(grid, start_x + 1, start_y + h - 1, w - 2, 0, tile);
        break;
    case 3:
        generate_random_tile(grid, start_x, start_y + 1, 0, h - 2, tile);
        break;
    default:
        l::error("generated invalid wall index, shouldn't happen");
    }
}

Error generate(Grid2D& grid, size_t n_rooms) {
    // TODO: create corridors
    // TODO: link corridors with doors by grouping them
    // TODO: add rectangle room shapes
    // TODO: challenge for circle, hexagon rooms https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm

    for (size_t i = 0; i < n_rooms; ++i) {
        size_t room_size { Random::generate(2, 4) };

        size_t room_x { 0 };
        size_t room_y { 0 };
        bool generating { true };
        size_t failed_attempts { 0 };
        // check if room has enough space to be put down
        while (generating && failed_attempts < 50) {
            room_x = Random::generate(1, (grid.width() - room_size - 1)); // rand room pos - check in bounds
            room_y = Random::generate(1, (grid.height() - room_size - 1));

            bool failed { false };
            // check if room fits
            // FIXME: if we fail to place 50 times, we still cram a room, wrongly
            for (size_t y = room_y; y < (room_y + room_size); ++y) {
                for (size_t x = room_x; x < (room_x + room_size); ++x) {
                    if (failed) {
                        break;
                    }
                    // generate only when empty
                    if (grid[x][y] != Tile::None) {
                        failed = true;
                        failed_attempts++;
                    }
                }
                if (failed) {
                    break;
                }
            }
            if (!failed) {
                generating = false;
            }
        }

        size_t wall_x = room_x - 1;
        size_t wall_y = room_y - 1;
        size_t wall_length = room_size + 2;

        fill_area(grid, wall_x, wall_y, wall_length, wall_length, Tile::NextToRoom);
        fill_area(grid, room_x, room_y, room_size, room_size, Tile::Room);
        fill_corners(grid, wall_x, wall_y, wall_length, wall_length, Tile::Corner);
        generate_doors(grid, wall_x, wall_y, wall_length, wall_length, Tile::Door);
    }

    size_t room_size { Random::generate(2, 4) };

    size_t corridor_start_x { 0 };
    size_t corridor_start_y { 0 };

    // attempt placing corridor start
    bool generating { true };
    size_t failed_attempts { 0 };
    while (generating && failed_attempts < 50) {
        corridor_start_x = Random::generate(1, (grid.width() - room_size - 1));
        corridor_start_y = Random::generate(1, (grid.height() - room_size - 1));

        bool failed { false };
        for (size_t y = corridor_start_y - 1; y < corridor_start_y + 1; y++) {
            for (size_t x = corridor_start_x - 1; x < corridor_start_x + 1; x++) {
                if (failed) {
                    break;
                }
                if (grid[x][y] != Tile::None) {
                    failed = true;
                    failed_attempts++;
                }

                grid[x][y] = Tile::Corridor;
            }
            if (failed) {
                break;
            }
        }
        if (!failed) {
            generating = false;
        }
    }

    return {};
}
