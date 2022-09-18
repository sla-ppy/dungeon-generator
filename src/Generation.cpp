#include "Generation.h"
#include "Log.h"

#include <random>

static inline std::random_device rd;
static inline std::mt19937 gen(rd());

int rand_gen(int max) {
    std::uniform_int_distribution<> distrib(0, max - 1); // here is where you can change the MIN and MAX values of the random dib.

    int rand_result = distrib(gen);
    return rand_result;
}

Error generate(Grid2D& grid) {
    // TODO: when randGen is getting used twice, we need another distribution of values, as we can have multiple and will need multiple
    // TODO: add diff. room sizes with rng
    // TODO: check if rooms overlap
    // FIXME: make sure next to room tiles are also in bounds, forgot where we guarantee that

    size_t room_size { 3 };
    for (int i = 0; i < 5; ++i) {
        // rand room pos
        const size_t pos_x = rand_gen(grid.width() - room_size);
        const size_t pos_y = rand_gen(grid.height() - room_size);

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

        bool has_door = false;
        if (has_door == false) {
            for (size_t y = 0; y < (pos_y + room_size); ++y) {
                for (size_t x = 0; x < (pos_x + room_size); ++x) {
                    if (grid[x][y] == Tile::NextToRoom) {
                        has_door = true;
                    }
                }
            }
        }
    }

    return {};
}
