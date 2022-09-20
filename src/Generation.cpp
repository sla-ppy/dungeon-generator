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

Error generate(Grid2D& grid) {
    // TODO: fix doors spawning on corners
    // TODO: create corridors
    // TODO: link corridors with doors by grouping them

    for (int i = 0; i < 5; ++i) {
        size_t room_size { Random::generate(2, 4) };

        size_t pos_x;
        size_t pos_y;
        bool generating { true };
        size_t failed_attempts { 0 };
        while (generating && failed_attempts < 50) {
            pos_x = Random::generate(1, (grid.width() - room_size - 1)); // rand room pos - check in bounds
            pos_y = Random::generate(1, (grid.height() - room_size - 1));

            bool failed { false };
            // check if room fits
            for (size_t y = pos_y; y < (pos_y + room_size); ++y) {
                for (size_t x = pos_x; x < (pos_x + room_size); ++x) {
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

        size_t starting_x = pos_x - 1;
        size_t starting_y = pos_y - 1;
        size_t outer_size = room_size + 2;

        for (size_t y = starting_y; y < starting_y + outer_size; ++y) {
            for (size_t x = starting_x; x < starting_x + outer_size; ++x) {
                grid[x][y] = Tile::NextToRoom;
            }
        }

        for (size_t y = pos_y; y < (pos_y + room_size); ++y) {
            for (size_t x = pos_x; x < (pos_x + room_size); ++x) {
                grid[x][y] = Tile::Room;
            }
        }

        size_t exact { 0 };
        size_t expected { Random::generate(1, 3) };
        for (size_t y = starting_y; y < (starting_y + outer_size); ++y) {
            for (size_t x = starting_x; x < (starting_x + outer_size); ++x) {
                if (grid[x][y] == Tile::NextToRoom && exact < expected) {
                    size_t chance { Random::generate(1, 100) };
                    if (chance < 10) {
                        grid[x][y] = Tile::Door;
                        exact++;
                    }
                }
            }
        }
    }

    return {};
}
