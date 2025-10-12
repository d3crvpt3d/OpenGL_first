#ifndef OPTIMIZE_BUFFER
#define OPTIMIZE_BUFFER

#include "chunkMap.h"
#include "main.h"
#include <vector>

typedef GLuint Block_t;

std::vector<Block_t> gen_optimized_buffer(Chunk_t &chunk);

#endif
