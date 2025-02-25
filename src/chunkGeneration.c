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

GLint ginstance_vbo;
GLint ginstance_vao;
GLint gcube_vbo;

//TODO: fix
//map chunk coord to actual memory via modulus RENDERSPAN
uint32_t *getChunkMemoryPosition(int32_t x, int32_t y, int32_t z){
	return NULL;
}


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

int inRenderRegion(vec3i_t pos, vec3i_t *currChunk){

	return
	abs(currChunk->x - pos.x) <= RENDERDISTANCE &&
	abs(currChunk->y - pos.y) <= RENDERDISTANCE &&
	abs(currChunk->z - pos.z) <= RENDERDISTANCE;
}

//TODO:
//check if chunk is stored somewhere else return NULL
uint8_t *getSavedChunk(vec3i_t pos){

	if(0){
		;
	}

	return NULL;
}

/* chunk gen thread(s) start */
//check for all chunks that are in renderdistance but not generated
//then generate Chunks if necessary
void *checkChunks(void *args){
	
	//cast args to vec3i_t for access to current Chunk coordinates
	vec3i_t *currChunk = (vec3i_t *) args;
	vec3i_t mem_pos = {0};

	//iterate over all chunks in memory
	GLint instanceVBO = ginstance_vbo;//load from global

	for(uint32_t z = 0; z < RENDERSPAN; z++){
		for(uint32_t y = 0; y < RENDERSPAN; y++){
			for(uint32_t x = 0; x < RENDERSPAN; x++){

				uint32_t *memory = getChunkMemoryPosition(x, y, z); //update memory position
				
				vec3i_t chunkPos = {
					currChunk->x - RENDERDISTANCE + x,
					currChunk->y - RENDERDISTANCE + y,
					currChunk->z - RENDERDISTANCE + z
				};

				//if chunk at xyz is not in renderdistance replace it with correct one
				if(!inRenderRegion(chunkPos, currChunk)){
					
					//check if chunk is saved then load, else generate chunk
					uint8_t *c_loaded = getSavedChunk(chunkPos);
				
					uint32_t offset = 0; //TODO: implement

					if(c_loaded){
						memcpy(memory, c_loaded, sizeof(uint8_t) * BLOCKS_PER_CHUNK);
					}else{
						generateChunk(memory, chunkPos);
					}

					
					//update shadow chunk on gpu TODO: swap with shadow buffer
					glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
					glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, BLOCKS_PER_CHUNK * sizeof(uint32_t), memory);
				}
			}
		}
	}

	//say that you finished and are ready to be joined
	gthreadDone = 1;
}

//spawn thread for generation chunks
pthread_t generateChunks(vec3i_t currChunkCoord){

	return pthread_create(&gchunkGenThread, NULL, checkChunks, &currChunkCoord);
}
