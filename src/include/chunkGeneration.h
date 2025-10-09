#ifndef CHUNK_GENERATION
#define CHUNK_GENERATION
#include <atomic>
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>

#include "chunkMap.h"

/* STRUCTS */
typedef struct vec3i_t{
	int32_t x;
	int32_t y;
	int32_t z;
} vec3i_t;

typedef struct Job {
	int32_t x;
	int32_t y;
	int32_t z;
	struct Job *nextJob;
} Job_t;

/* GLOBALS */
extern uint8_t programRunning;
extern Job_t *lastJob;
extern Job_t *jobQueue;
extern ChunkMap_t *chunkMap;
extern vec3i_t currChunk;
extern std::atomic_uint8_t update_shadowVBO;

/* FUNCTIONS */

//use at start of program to generate the chunk gen threads
void setUpThreads();
void clearThreads();

void addJob(int32_t x, int32_t y, int32_t z);
void addNewChunkJobs(int32_t lastX, int32_t lastY, int32_t lastZ, int32_t currX, int32_t currY, int32_t currZ);
uint8_t inRenderRegion(Chunk_t *chunk);

#endif
