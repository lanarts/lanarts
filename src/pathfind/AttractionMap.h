#pragma once

#include <vector>

#include <lcommon/geometry.h>
#include <lcommon/Grid.h>

#include "BoolGridRef.h"
#include "FloodFillPaths.h"

class AttractionMap {
public:
    void init(const BoolGridRef& solidity) {
        _solidity = solidity;
        attraction_map.resize(solidity->size());
    }

    struct Cache { // Data that can be freed if needed, not serialized
        FloodFillPaths paths_from_source;
    };

    void static_repell(Pos source_xy);
    Cache _cache;
    /* Shared with game tile structure */
    BoolGridRef _solidity;

    Grid<short> attraction_map;
};

