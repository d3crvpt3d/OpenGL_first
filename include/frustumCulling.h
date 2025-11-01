#pragma once

#include "chunkGeneration.h"
#include <cstdint>
bool outOfFrustum(vec3i_t currChunk, float fov_view_dot, int32_t lx, int32_t ly, int32_t lz, float viewX, float viewY, float viewZ);
