#ifndef CHUNK_GENERATION
#define CHUNK_GENERATION
#include <condition_variable>
#include <mutex>
#include <pthread.h>
#include <queue>
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
extern std::queue<vec3i_t> jobQueue;
extern vec3i_t currChunk;
extern vec3i_t lastChunk;
extern std::queue<vec3i_t> genChunksQueue; //queue of chunks ready to send to VRAM

extern std::mutex jobMutex;
extern std::mutex genChunksQueue_mutex;

extern std::condition_variable updateThreadCV;

extern void *face_offset[2][6];

/* FUNCTIONS */

//use at start of program to generate the chunk gen threads
void setUpThreads();
void clearThreads();

void addJob(int32_t x, int32_t y, int32_t z);
void addNewChunkJobs(int32_t lastX, int32_t lastY, int32_t lastZ, int32_t currX, int32_t currY, int32_t currZ);
uint8_t inRenderRegion(Chunk_t *chunk);

void checkIfNewChunk(float xyz[3]);
#endif
