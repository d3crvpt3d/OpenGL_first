#pragma once
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <glad/gl.h>
#include <math.h>
#include <GLFW/glfw3.h>


#define RENDERDISTANCE 2
#define CHUNK_WD 64	//chunk width/depth
#define CHUNK_H 16	//chunk height
#define BLOCKS_PER_CHUNK (CHUNK_WD * CHUNK_WD * CHUNK_H)
#define CHUNK_THREADS 4

#define RENDERSPAN (2*RENDERDISTANCE+1) //from x-renderdistance to x+renderdistance in chunks
#define CHUNKS RENDERSPAN * RENDERSPAN * RENDERSPAN

extern GLint ginstance_vbo;
extern GLint ginstance_vao;
extern GLint gcube_vbo;

typedef struct vec3i_t{
	int32_t x;
	int32_t y;
	int32_t z;
} vec3i_t;

typedef struct Chunk{
	uint32_t blocks[BLOCKS_PER_CHUNK];
	vec3i_t pos;
	uint8_t active_shadow;
} Chunk_t;

extern Chunk_t grenderRegion[CHUNKS];


extern pthread_t chunkGenThread;
extern _Atomic uint8_t threadDone;



//generate [size] Chunks for each Chunk [coord]
pthread_t generateChunks(vec3i_t currChunkCoord);

uint8_t *getChunkMemoryPosition(int32_t x, int32_t y, int32_t z);
