#include "Common.h"
#include <fmt/core.h>

#include "Rendering.h"
#include "Generation.h"

int main() {
    Array2D<Tile, 20, 20> grid;
    grid.fill(Tile::None);
    
    auto err = generate(grid);
    if (err) {
        fmt::print("error: failed to generate: {}\n", err.msg);
        return 1;
    }
    
    err = render(grid);
    if (err) {
        fmt::print("error: failed to render: {}\n", err.msg);
        return 1;
    }
}
