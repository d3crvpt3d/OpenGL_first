#pragma once
#include "chunkGeneration.h"

void update_lookingAt(float xyz[3], float yaw_pitch[2], vec3i_t *break_block, vec3i_t *place_block, float maxDistance, ChunkMap_t *chunkMap);