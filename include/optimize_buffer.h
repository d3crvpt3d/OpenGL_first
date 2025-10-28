#ifndef OPTIMIZE_BUFFER
#define OPTIMIZE_BUFFER

#include "chunkMap.h"
#include "main.h"
#include <array>
#include <cstdint>
#include <vector>

typedef struct {
	GLfloat xyz[3];	//pos
	GLuint size[2];	//width/height
	GLushort type;	//type
} QuadGPU_t;

typedef struct{
	int32_t x;
	int32_t y;
	int32_t z;
	std::array<std::vector<QuadGPU_t>, 6> data;
} BufferCache_t;

std::array<std::vector<QuadGPU_t>, 6> gen_optimized_buffer(
		ChunkMap &map,
		int32_t cx,
		int32_t cy,
		int32_t cz,
		int32_t px,
		int32_t py,
		int32_t pz);

//log2 of unsigned integer
uint32_t log2u(uint32_t num);
uint32_t lod_number(int32_t x, int32_t y, int32_t z);
uint32_t lod_index(int32_t x, int32_t y, int32_t z);
#endif
