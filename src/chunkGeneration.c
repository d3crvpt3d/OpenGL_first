#include "chunkGeneration.h"

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

RenderRegion renderRegion = {0};
uint32_t seed = 0; //currently 0

//TODO: check if really works
//map chunk coord to actual memory via modulus RENDERSPAN
void getChunkMemoryPosition(vec3i_t *dest, int32_t x, int32_t y, int32_t z){
	dest->x = (x + RENDERDISTANCE) % RENDERSPAN;
	dest->y = (y + RENDERDISTANCE) % RENDERSPAN;
	dest->z = (z + RENDERDISTANCE) % RENDERSPAN;
}


//generate chunk on chunk coord [pos]
void generateChunk(vec3i_t *memoryDest, vec3i_t *pos){
	srand(seed); //regenerate random values
	Chunk *chunk = &renderRegion.chunk[memoryDest->z][memoryDest->y][memoryDest->x];
	
	for(uint8_t z = 0; z < CHUNK_WDH; z++){
		for(uint8_t y = 0; y < CHUNK_WDH; y++){
			for(uint8_t x = 0; x < CHUNK_WDH; x++){
				chunk->block[z][y][x] = (uint8_t) rand();
			}
		}
	}
}

int inRenderRegion(vec3i_t *pos, vec3i_t *currChunk){

	return
	abs(currChunk->x - pos->x) <= RENDERDISTANCE &&
	abs(currChunk->y - pos->y) <= RENDERDISTANCE &&
	abs(currChunk->z - pos->z) <= RENDERDISTANCE;
}

//TODO:
//check if chunk is stored somewhere else return NULL
Chunk *getSavedChunk(vec3i_t pos){

	if(0){
		;
	}

	return NULL;
}

/* chunk gen thread(s) start */
//check for all chunks that are in renderdistance but not generated
//then generate Chunks if necessary
checkChunks(void *args){
	
	//cast args to vec3i_t for access to current Chunk coordinates
	vec3i_t *currChunk = (vec3i_t *) args;
	vec3i_t min = {currChunk->x-RENDERDISTANCE, currChunk->y-RENDERDISTANCE, currChunk->z-RENDERDISTANCE};
	vec3i_t mem_pos = {0};

	//iterate over all chunks in memory
	for(uint32_t z = 0; z < RENDERSPAN; z++){
		for(uint32_t y = 0; y < RENDERSPAN; y++){
			for(uint32_t x = 0; x < RENDERSPAN; x++){

				getChunkMemoryPosition(&mem_pos, x, y, z); //update memory position
				
				vec3i_t chunkPos = {
					currChunk->x - RENDERDISTANCE + x,
					currChunk->y - RENDERDISTANCE + y,
					currChunk->z - RENDERDISTANCE + z
				};

				//if chunk at xyz is not in renderdistance replace it with correct one
				if(!inRenderRegion(&mem_pos, currChunk)){
					
					//check if chunk is saved then load, else generate chunk
					Chunk *c_loaded = getSavedChunk(mem_pos);
				
					if(c_loaded){
						memcpy(&renderRegion.chunk[mem_pos.z][mem_pos.y][mem_pos.x], c_loaded, sizeof(uint8_t) * BLOCKS_PER_CHUNK);
					}else{
						generateChunk(&mem_pos, &chunkPos);
					}

				}
			}
		}
	}
	//say that you finished and are ready to be joined
	threadDone = 1;
}

//spawn thread for generation chunks
pthread_t generateChunks(vec3i_t currChunkCoord){

	return pthread_create(&chunkGenThread, NULL, checkChunks, &currChunkCoord);
}