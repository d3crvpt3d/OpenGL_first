#include "chunkMap.h"

uint32_t modulo(int32_t a, int32_t b){
	return ((a % b) + b) % b;
}

ChunkMap_t *chunkMap_init(uint8_t wdh){
	ChunkMap_t *tmp = (ChunkMap_t *) calloc(1, sizeof(ChunkMap_t));
	tmp->wdh = wdh;
	tmp->chunks = (Chunk_t *) calloc(wdh*wdh*wdh, sizeof(Chunk_t));
	return tmp;
}

Chunk_t *chunkMap_get(ChunkMap_t *chunkMap, int32_t x, int32_t y, int32_t z){
	return &chunkMap->chunks[
		modulo(z, RENDERSPAN) * RENDERSPAN * RENDERSPAN +
		modulo(y, RENDERSPAN) * RENDERSPAN +
		modulo(x, RENDERSPAN)
	];
}

void chunkMap_destroy(ChunkMap_t *chunkMap){
	free(chunkMap->chunks);
	free(chunkMap);
}