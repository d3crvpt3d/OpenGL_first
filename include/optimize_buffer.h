#ifndef OPTIMIZE_BUFFER
#define OPTIMIZE_BUFFER

#include "chunkMap.h"
#include "main.h"
#include <vector>

typedef struct {
	GLfloat xyz[3];
	GLuint type;
} BlockGPU_t;

std::vector<BlockGPU_t> gen_optimized_buffer(Chunk_t &chunk);

#endif
