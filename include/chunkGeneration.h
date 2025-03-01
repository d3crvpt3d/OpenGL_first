#pragma once
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <glad/gl.h>
#include <math.h>
#include <GLFW/glfw3.h>

#include "chunkTree.h"

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
	int16_t x;
	int16_t y;
	int16_t z;
} vec3i_t;

typedef struct Job {
	int16_t x;
	int16_t y;
	int16_t z;
	struct Job *nextJob;
} Job_t;

//a Chunk in CPU memory is a 3D uint16_t array where each int is the block type
typedef struct Chunk{
	uint16_t blocks[2][CHUNK_WDH][CHUNK_WDH][CHUNK_WDH];
	uint8_t activeBuffer: 1; //for shadow buffer
	int16_t x;
	int16_t y;
	int16_t z;
} Chunk_t;


/* FUNCTIONS */

//use at start of program to generate the chunk gen threads
void setUpThreads();
void clearThreads();

void addJob(int16_t x, int16_t y, int16_t z);
void removeChunk(int16_t x, int16_t y, int16_t z);
void addNewChunkJobs(int16_t lastX, int16_t lastY, int16_t lastZ, int16_t currX, int16_t currY, int16_t currZ);
void removeChunks(int16_t lastX, int16_t lastY, int16_t lastZ, int16_t currX, int16_t currY, int16_t currZ);