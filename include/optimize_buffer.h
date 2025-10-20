#ifndef OPTIMIZE_BUFFER
#define OPTIMIZE_BUFFER

#include "chunkMap.h"
#include "main.h"
#include <vector>

typedef struct {
	GLfloat xyz[3];	//pos
	GLuint size[2];	//width/height
	GLushort type;	//type
} QuadGPU_t;

std::array<std::vector<QuadGPU_t>, 6> gen_optimized_buffer(ChunkMap &map, int32_t x, int32_t y, int32_t z);

#endif
