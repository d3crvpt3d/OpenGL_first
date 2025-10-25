#ifndef OPTIMIZE_BUFFER
#define OPTIMIZE_BUFFER

#include "chunkMap.h"
#include "main.h"
#include <cstdint>
#include <vector>

typedef struct {
	GLfloat xyz[3];	//pos
	GLuint size[2];	//width/height
	GLushort type;	//type
} QuadGPU_t;

std::array<std::vector<QuadGPU_t>, 6> gen_optimized_buffer(
		ChunkMap &map,
		int32_t cx,
		int32_t cy,
		int32_t cz,
		int32_t px,
		int32_t py,
		int32_t pz);

#endif
