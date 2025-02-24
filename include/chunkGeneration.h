#pragma once
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <GLFW/glfw3.h>


#define CHUNK_WD 64	//chunk width/depth
#define CHUNK_H 16	//chunk height
#define BLOCKS_PER_CHUNK CHUNK_WD * CHUNK_WD * CHUNK_H
#define RENDERDISTANCE 2
#define CHUNK_THREADS 4

#define RENDERSPAN (2*RENDERDISTANCE+1) //from x-renderdistance to x+renderdistance
#define CHUNKS RENDERSPAN * RENDERSPAN * RENDERSPAN

extern pthread_t chunkGenThread;
extern _Atomic uint8_t threadDone;

typedef struct{
	int32_t x;
	int32_t y;
	int32_t z;
} vec3i_t;

//to check if draw: block[z][y][x] && visible
//raw chunk where each block is uint as blocktype (0 := air, so that "if(block[x][y][z])" works)
typedef struct{
	vec3i_t pos;
	uint8_t visible[6];//visible faces
	uint32_t numBlocks;
	uint8_t block[BLOCKS_PER_CHUNK];//chunk_wdh^3
} Chunk;

//holds the actual raw chunk data
typedef struct{
	Chunk chunk[RENDERSPAN][RENDERSPAN][RENDERSPAN];
} RenderRegion;

extern RenderRegion renderRegion;

typedef struct{
	uint32_t length;
	vec3i_t *coord;
} ChunkQueue;

//generate [size] Chunks for each Chunk [coord]
pthread_t generateChunks(vec3i_t currChunkCoord);

void getChunkMemoryPosition(vec3i_t *dest, int32_t x, int32_t y, int32_t z);
