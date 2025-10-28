#pragma once

#include <array>
#include <cstdint>

#include "chunkMap.h"
#include "main.h"

typedef struct {
	int32_t x;
	int32_t y;
	int32_t z;
	uint64_t count[6];
	uint64_t offset[6];
	GLuint instanceVBO;
} ChunkCPU_t;

class BufferMap{

	std::array<
		std::array<
			std::array<
				ChunkCPU_t,
			RENDERSPAN>,
		RENDERSPAN>,
	RENDERSPAN> data = {0};

	public:
		ChunkCPU_t *at(int32_t x, int32_t y, int32_t z){
			return &data.at(mod(z, RENDERSPAN))
						.at(mod(y, RENDERSPAN))
						.at(mod(x, RENDERSPAN));
		}

};
