#include "Common.h"
#include <fmt/core.h>

#include "Generation.h"
#include "Log.h"
#include "Rendering.h"

int main() {
    Array2D<Tile, 20, 20> grid;
    grid.fill(Tile::None);

    auto err = generate(grid);
    if (err) {
        l::error("failed to generate: {}\n", err.msg);
        return 1;
    }

    // TODO: choose rendering mode :-D

    const std::string output_file = "output";

    err = render(grid, output_file, 10);
    if (err) {
        l::error("failed to render: {}\n", err.msg);
        return 1;
    }
}
