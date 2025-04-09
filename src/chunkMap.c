#include "chunkMap.h"

//b has to be in 2^n
uint32_t modulo_n2(int32_t a, int32_t b){
	return a & (b-1);
}

uint32_t mod(int32_t a, int32_t b){
	return ((a % b) + b) % b;
}

ChunkMap_t *chunkMap_init(uint8_t wdh){
	ChunkMap_t *tmp = (ChunkMap_t *) calloc(1, sizeof(ChunkMap_t));
	tmp->wdh = wdh;
	tmp->chunks = (Chunk_t *) calloc(wdh*wdh*wdh, sizeof(Chunk_t));
	return tmp;
}

uint16_t chunkMap_getBlock(ChunkMap_t *chunkMap, int32_t x, int32_t y, int32_t z){
	Chunk_t *chunk = chunkMap_getChunk(chunkMap, x >> 6, y >> 6, z >> 6);
	return chunk->blocks[z & 63][y & 63][x & 63];
}

Chunk_t *chunkMap_getChunk(ChunkMap_t *chunkMap, int32_t x, int32_t y, int32_t z){
	return &chunkMap->chunks[
		mod(z, RENDERSPAN) * RENDERSPAN * RENDERSPAN +
		mod(y, RENDERSPAN) * RENDERSPAN +
		mod(x, RENDERSPAN)
	];
}

void chunkMap_destroy(ChunkMap_t *chunkMap){
	free(chunkMap->chunks);
	free(chunkMap);
}