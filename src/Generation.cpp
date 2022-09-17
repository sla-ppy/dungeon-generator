#include "Generation.h"

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
    // number of rooms placed depends on rng?

    // TODO: add diff. room sizes with rng
    int room_size{7};
    for (int i = 0; i < 5; ++i) {
        int pos_x = rand_gen(grid.width() - room_size);
        int pos_y = rand_gen(grid.height() - room_size);

        for (int x = pos_x; x < (pos_x + room_size); ++x) {
            for (int y = pos_y; y < (pos_y + room_size); ++y) {
                grid[x][y] = Tile::Room;
            }
        }
    }

    return {};
}
