#include "chunkGeneration.h"

pthread_t gchunkGenThread = 0;
_Atomic uint8_t gthreadDone = 1;


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

Chunk_t grenderRegion[CHUNKS] = {0};
uint32_t gseed = 0; //currently 0


//generate chunk on chunk coord [pos]
void generateChunk(uint32_t *chunk_mem, vec3i_t chunk_coord){
	srand(gseed); //regenerate random values

	vec3i_t global = {chunk_coord.x * CHUNK_WD, chunk_coord.y * CHUNK_H, chunk_coord.z * CHUNK_WD};

	for(uint8_t z = 0; z < CHUNK_WD; z++){
		for(uint8_t y = 0; y < CHUNK_H; y++){
			for(uint8_t x = 0; x < CHUNK_WD; x++){
				
				//sin wave generator with amplitude: 5, freq: 1/5
				if(y == (int32_t) (5.0f * sin( (float) (global.x+x) / 5.0f ))){
					chunk_mem[CHUNK_WD * CHUNK_H * z + CHUNK_WD * y + x] = 1;
				}else{
					chunk_mem[CHUNK_WD * CHUNK_H * z + CHUNK_WD * y + x] = 0;
				}

			}
		}
	}
}

void *checkChunks(){


	gthreadDone = 1;
}

//spawn thread for generation chunks
pthread_t generateChunks(vec3i_t currChunkCoord){
	gthreadDone = 0;
	return pthread_create(&gchunkGenThread, NULL, checkChunks, &currChunkCoord);
}
