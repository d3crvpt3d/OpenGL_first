#pragma once
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <GLFW/glfw3.h>


#define CHUNK_WDH 8
#define RENDERDISTANCE 10
#define CHUNK_THREADS 4
#define CHUNKS (2*RENDERDISTANCE+1) * (2*RENDERDISTANCE+1)

typedef struct {
	GLuint data[CHUNK_WDH][CHUNK_WDH][CHUNK_WDH];
} Chunk;

typedef struct{
	uint32_t x;
	uint32_t y;
	uint32_t z;
} ChunkCoord;

typedef struct {
	pthread_t* threads;
	uint32_t thread_count;
} ThreadPool;

void generateChunks(int32_t x, int32_t y, int32_t z, Chunk *chunks);