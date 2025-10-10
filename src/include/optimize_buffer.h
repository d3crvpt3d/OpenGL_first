#ifndef OPTIMIZE_BUFFER
#define OPTIMIZE_BUFFER

#include "chunkMap.h"
#include "main.h"
#include <vector>

typedef struct{
	GLuint x:6;
	GLuint y:6;
	GLuint z:6;
	GLuint break_progress: 4;
	GLuint type: 10;
} Block_t;

std::vector<Block_t> gen_optimized_buffer(Chunk_t &chunk);

#endif
