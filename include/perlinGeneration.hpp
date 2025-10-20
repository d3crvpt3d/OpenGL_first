#pragma once

#include "chunkMap.h"

void generate_chunk_with_caves(
    ChunkMap &map,
    int chunk_x,
    int chunk_y,
    int chunk_z,
    int octaves = 4,
    int sample_rate = 4);
