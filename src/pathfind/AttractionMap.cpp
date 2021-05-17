#include "AttractionMap.h"

void AttractionMap::static_repell(Pos center_xy) {
    FloodFillPaths& paths = _cache.paths_from_source;
    paths.initialize(_solidity);

    // Should generate from center as much as possible,
    // but should also make sure each square is filled

    BBox region = {center_xy - Pos(5, 5), Size(10,10)};
    region = region.resized_within({Pos(), _solidity->size()});
    paths.fill_paths_tile_region(center_xy - region.left_top(), region);

    FOR_EACH_BBOX(region, x, y) {
        Pos tile_xy = {x, y};
        FloodFillNode* node = paths.node_at(tile_xy);
        attraction_map[tile_xy] -= 100 - (short)node->distance;
    }
}
