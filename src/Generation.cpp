#include "Generation.h"
#include "Log.h"

#include <random>

namespace Random {
uint16_t generate(uint16_t min, uint16_t max) {
    static std::mt19937 mt { std::random_device {}() };
    std::uniform_int_distribution set { min, max };
    return set(mt);
}
}

Error generate(Grid2D& grid) {
    // TODO: add diff. room sizes with rng
    // TODO: check if rooms overlap
    // FIXME: make sure next to room tiles are also in bounds, forgot where we guarantee that

    size_t room_size { 3 };
    for (int i = 0; i < 5; ++i) {
        // rand room pos
        const size_t pos_x = Random::generate(0, (grid.width() - room_size));
        const size_t pos_y = Random::generate(0, (grid.height() - room_size));

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

        for (size_t y = 0; y < (pos_y + room_size); ++y) {
            for (size_t x = 0; x < (pos_x + room_size); ++x) {
                if (grid[x][y] == Tile::NextToRoom) {
                    grid[x][y] = Tile::Door;
                }
            }
        }
    }

    return {};
}
