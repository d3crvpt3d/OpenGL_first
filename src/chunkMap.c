#include "chunkMap.h"

ChunkMap_t *chunkMap_init(uint8_t wdh){
	ChunkMap_t *tmp = (ChunkMap_t *) calloc(1, sizeof(ChunkMap_t));
	tmp->wdh = wdh;
	tmp->chunks = (Chunk_t *) calloc(CHUNKS, sizeof(Chunk_t));
	return tmp;
}

Chunk_t *chunkMap_get(ChunkMap_t *chunkMap, int32_t x, int32_t y, int32_t z){
	return &chunkMap->chunks[
		(z % RENDERSPAN) * RENDERSPAN * RENDERSPAN +
		(y % RENDERSPAN) * RENDERSPAN +
		(x % RENDERSPAN)
	];
}

void chunkMap_destroy(ChunkMap_t *chunkMap){
	free(chunkMap->chunks);
	free(chunkMap);
}