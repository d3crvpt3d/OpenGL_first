#pragma once
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <glad/gl.h>
#include <math.h>
#include <GLFW/glfw3.h>

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

//a Chunk in CPU memory is a 3D uint16_t array where each int is the block type
typedef struct Chunk{
	uint16_t blocks[CHUNK_WDH][CHUNK_WDH][CHUNK_WDH];
} Chunk_t;

//bit arrays for binary meshing
typedef struct xBlocks{
	uint64_t bitRow[CHUNK_WDH][CHUNK_WDH];
} xBlocks_t;

typedef struct yBlocks{
	uint64_t bitRow[CHUNK_WDH][CHUNK_WDH];
} yBlocks_t;

typedef struct zBlocks{
	uint64_t bitRow[CHUNK_WDH][CHUNK_WDH];
} zBlocks_t;


/* FUNCTIONS */

//use at start of program to generate the chunk gen threads
void setUpThreads();
void clearThreads();

//generate [size] Chunks for each Chunk [coord]
pthread_t generateChunks(vec3i_t currChunkCoord);

void addJob(int32_t x, int32_t y, int32_t z);