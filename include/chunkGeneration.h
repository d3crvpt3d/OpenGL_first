#pragma once
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <GLFW/glfw3.h>


#define CHUNK_WDH 8
#define RENDERDISTANCE 10
#define CHUNK_THREADS 4
#define BLOCKS_PER_CHUNK CHUNK_WDH * CHUNK_WDH * CHUNK_WDH

#define RENDERSPAN (2*RENDERDISTANCE+1) //from x-renderdistance to x+renderdistance
#define CHUNKS RENDERSPAN * RENDERSPAN * RENDERSPAN

//raw chunk where each block is uint as blocktype (0 := air, so that "if(block[x][y][z])" works)
typedef struct{
	vec3i_t pos;
	uint8_t block[CHUNK_WDH][CHUNK_WDH][CHUNK_WDH];
} Chunk;

//holds the actual raw chunk data
typedef struct{
	Chunk chunk[RENDERSPAN][RENDERSPAN][RENDERSPAN];
} RenderRegion;

typedef struct{
	int32_t x;
	int32_t y;
	int32_t z;
} vec3i_t;

typedef struct{
	uint32_t length;
	vec3i_t *coord;
} ChunkQueue;

//generate [size] Chunks for each Chunk [coord]
void generateChunks(vec3i_t currChunkCoord);

void getChunkMemoryPosition(vec3i_t *dest, vec3i_t *coord);