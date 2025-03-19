#include "chunkMap.h"

//b has to be in 2^n
uint32_t modulo_n2(int32_t a, int32_t b){
	return a & (b-1);
}

ChunkMap_t *chunkMap_init(uint8_t wdh){
	ChunkMap_t *tmp = (ChunkMap_t *) calloc(1, sizeof(ChunkMap_t));
	tmp->wdh = wdh;
	tmp->chunks = (Chunk_t *) calloc(wdh*wdh*wdh, sizeof(Chunk_t));
	return tmp;
}

Chunk_t *chunkMap_get(ChunkMap_t *chunkMap, int32_t x, int32_t y, int32_t z){
	return &chunkMap->chunks[
		modulo_n2(z, RENDERSPAN) * RENDERSPAN * RENDERSPAN +
		modulo_n2(y, RENDERSPAN) * RENDERSPAN +
		modulo_n2(x, RENDERSPAN)
	];
}

void chunkMap_destroy(ChunkMap_t *chunkMap){
	free(chunkMap->chunks);
	free(chunkMap);
}