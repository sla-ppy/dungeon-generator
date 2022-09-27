#include "Common.h"
#include <fmt/core.h>

#include "Generation.h"
#include "Log.h"
#include "Rendering.h"

int main() {
    Grid2D grid;
    grid.fill(Tile::None);

    auto err = generate(grid, 5);
    if (err) {
        l::error("failed to generate: {}\n", err.msg);
        return 1;
    }

    // TODO: choose rendering mode :-D

    const std::string output_file = "output";

    err = render(grid, output_file, 32, true);
    if (err) {
        l::error("failed to render: {}\n", err.msg);
        return 1;
    }
}
