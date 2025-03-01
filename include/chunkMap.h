#pragma once
#include "chunkGeneration.h"

/* STRUCTS */



typedef struct ChunkMap{
	uint16_t wdh;
	Chunk_t *chunks;
} ChunkMap_t;

/* FUNCTIONS */

//malloc chunkMap
ChunkMap_t *chunkMap_init(uint8_t wdh);

//get pointer to memory of chunk at (x,y,z)
Chunk_t *chunkMap_get(ChunkMap_t *chunkMap, int32_t x, int32_t y, int32_t z);

//free chunkMap
void chunkMap_destroy(ChunkMap_t *chunkMap);