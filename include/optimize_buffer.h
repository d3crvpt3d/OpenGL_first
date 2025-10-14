#ifndef OPTIMIZE_BUFFER
#define OPTIMIZE_BUFFER

#include "chunkMap.h"
#include "main.h"
#include <vector>

typedef struct {
	GLfloat xyz[3];
	GLuint type;
} BlockGPU_t;

static_assert(sizeof(BlockGPU_t) == 16, "BlockGPU_t must be 16 bytes!");
static_assert(offsetof(BlockGPU_t, xyz) == 0, "xyz offset must be 0");
static_assert(offsetof(BlockGPU_t, type) == 12, "type offset must be 12");

std::vector<BlockGPU_t> gen_optimized_buffer(Chunk_t &chunk);

#endif
