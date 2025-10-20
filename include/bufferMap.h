//cache for optimized per chunk
//instance data with modular indexing
#pragma once

#include "chunkMap.h"
#include "optimize_buffer.h"
#include <array>
#include <cstdint>
#include <vector>

//stores face data for each side
typedef struct BufferCache{
	std::array<std::vector<QuadGPU_t>, 6> data;
	int32_t x;
	int32_t y;
	int32_t z;
	bool initialized = false;
} BufferCache_t;

class BufferMap{

	std::array<
		std::array<
			std::array<
				BufferCache_t,
			CHUNK_WDH>,
		CHUNK_WDH >,
	CHUNK_WDH> buffers;

	public:
		std::array<std::vector<QuadGPU_t>, 6> *at(int32_t x, int32_t y, int32_t z){
			BufferCache_t *check = &buffers
				.at(mod(z, CHUNK_WDH))
				.at(mod(y, CHUNK_WDH))
				.at(mod(x, CHUNK_WDH));

			//check if right buffer
			if(	check->x == x &&
				check->y == y &&
				check->z == z ){
				return &check->data;
			}

			//if not return nullptr
			return nullptr;
		}

};
