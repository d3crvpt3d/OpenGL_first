#pragma once
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <GLFW/glfw3.h>

//get num of processors
#ifdef _WIN32
	//Windows
	
	#include <windows.h>
	
	uint32_t get_max_threads(){
		SYSTEM_INFO sysinfo;
		GetSystemInfo(&sysinfo);
		return sysinfo.dwNumberOfProcessors;
	}
#elif defined(__APPLE__) || defined(__linux__)
	#include <unistd.h>

	uint32_t get_max_threads() {
		long n_processors = sysconf(_SC_NPROCESSORS_ONLN);
		return (uint32_t)n_processors;
	}
#else
	#error "unsupported platform"
#endif


#define CHUNK_WDH 8
#define RENDERDISTANCE 10
#define CHUNK_THREADS 4

typedef struct {
	GLuint data[CHUNK_WDH][CHUNK_WDH][CHUNK_WDH];
} Chunk;

typedef struct {

} World;

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