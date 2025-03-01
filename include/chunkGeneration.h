#pragma once
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <glad/gl.h>
#include <math.h>
#include <GLFW/glfw3.h>

#include "chunkMap.h"

/* DEFINES */
#define RENDERDISTANCE 2
#define CHUNK_WDH 64	//chunk width/depth/height
#define BLOCKS_PER_CHUNK (CHUNK_WDH * CHUNK_WDH * CHUNK_WDH)
#define CHUNK_THREADS 4

#define RENDERSPAN (2*RENDERDISTANCE+1) //from x-renderdistance to x+renderdistance in chunks
#define CHUNKS RENDERSPAN * RENDERSPAN * RENDERSPAN


/* GLOBALS */
extern uint8_t programRunning;
extern Job_t *lastJob;
extern Job_t *jobQueue;

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

//a Chunk in CPU memory is a 3D uint32_t array where each int is the block type
typedef struct Chunk{
	int16_t blocks[2][CHUNK_WDH][CHUNK_WDH][CHUNK_WDH];
	uint8_t activeBuffer: 1; //for shadow buffer
	uint8_t modified: 1;
	int32_t x;
	int32_t y;
	int32_t z;
} Chunk_t;


/* FUNCTIONS */

//use at start of program to generate the chunk gen threads
void setUpThreads();
void clearThreads();

void addJob(int32_t x, int32_t y, int32_t z);
void addNewChunkJobs(int32_t lastX, int32_t lastY, int32_t lastZ, int32_t currX, int32_t currY, int32_t currZ);
uint8_t inRenderRegion(Chunk_t *chunk);