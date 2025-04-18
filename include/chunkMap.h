#pragma once

#include <stdlib.h>
#include <stdint.h>


/* DEFINES */
#define RENDERDISTANCE 2
#define CHUNK_WDH 64	//chunk width/depth/height
#define BLOCKS_PER_CHUNK (CHUNK_WDH * CHUNK_WDH * CHUNK_WDH)
#define CHUNK_THREADS 4

#define RENDERSPAN (2*RENDERDISTANCE+1) //from x-renderdistance to x+renderdistance in chunks
#define CHUNKS RENDERSPAN * RENDERSPAN * RENDERSPAN

/* STRUCTS */

//a Chunk in CPU memory is a 3D uint32_t array where each int is the block type
typedef struct Chunk{
	int16_t blocks[CHUNK_WDH][CHUNK_WDH][CHUNK_WDH];
	uint8_t modified: 1;
	uint8_t initialized: 1;
	int32_t x;
	int32_t y;
	int32_t z;
} Chunk_t;

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