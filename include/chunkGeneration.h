#pragma once
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <glad/gl.h>
#include <math.h>
#include <GLFW/glfw3.h>


#define CHUNK_WD 64	//chunk width/depth
#define CHUNK_H 16	//chunk height
#define BLOCKS_PER_CHUNK CHUNK_WD * CHUNK_WD * CHUNK_H
#define RENDERDISTANCE 2
#define CHUNK_THREADS 4

#define RENDERSPAN (2*RENDERDISTANCE+1) //from x-renderdistance to x+renderdistance in chunks
#define CHUNKS RENDERSPAN * RENDERSPAN * RENDERSPAN

extern GLint instance_vbo;
extern GLint instance_vao;
extern GLint cube_vbo;
extern uint8_t renderRegion[CHUNKS * BLOCKS_PER_CHUNK];


extern pthread_t chunkGenThread;
extern _Atomic uint8_t threadDone;

typedef struct{
	int32_t x;
	int32_t y;
	int32_t z;
} vec3i_t;

//generate [size] Chunks for each Chunk [coord]
pthread_t generateChunks(vec3i_t currChunkCoord);

uint8_t *getChunkMemoryPosition(int32_t x, int32_t y, int32_t z);
