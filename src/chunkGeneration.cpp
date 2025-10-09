#include "include/chunkGeneration.h"

#include <atomic>
#include <stdio.h>

#define CHUNKDATA E:\Code\Projects\OpenGL\opengl_glfw_1\include\main.h
#define PI 3.14159265358979323846f


uint8_t programRunning = 1;
vec3i_t currChunk = {0, 0, 0};
ChunkMap_t *chunkMap = NULL;

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

uint32_t gseed = 0; //currently 0
std::atomic_uint8_t update_shadowVBO = 0; //set up main thread to update shadow vbo and swap buffers

pthread_t chunkQueue[CHUNK_THREADS];


//linked list
Job_t *jobQueue = NULL;
Job_t *lastJob = NULL;

pthread_mutex_t jobMutex;

void addJob(int32_t x, int32_t y, int32_t z){
	pthread_mutex_lock(&jobMutex);
	Job_t *lastlastJob = lastJob;
	lastJob = (Job_t *) malloc(sizeof(Job_t));
	if(lastlastJob){
		lastlastJob->nextJob = lastJob;
	}
	if(!jobQueue){
		jobQueue = lastJob;
	}
	lastJob->x = x;
	lastJob->y = y;
	lastJob->z = z;
	lastJob->nextJob = NULL;
	pthread_mutex_unlock(&jobMutex);
}

void chunkFunction(Chunk_t *chunk){

	//debug
	//memset(chunk->blocks[0][0], (short) 123, sizeof(short) * 63);
	//return;
	
	
	float x0 = chunk->x * CHUNK_WDH;
	float y0 = chunk->y * CHUNK_WDH;
	float z0 = chunk->z * CHUNK_WDH;

	for(int32_t z = z0; z < z0+CHUNK_WDH; z++){
		for(int32_t y = y0; y < y0+CHUNK_WDH; y++){
			for(int32_t x = x0; x < x0+CHUNK_WDH; x++){
				chunk->blocks[z][y][x] = (uint16_t) (3.9999f / (1.f + expf(.5f * (float) (y - 30) - 2.f * sinf( ((float) x) / 10.f) * cosf( ((float) z) / 10.f) ) ) );
				//fprintf(stderr, "%d,%d,%d: %f\n", x, y, z, chunk->blocks[z][y][x]);
			}
		}
	}
}

//generate chunk on chunk coord [pos]
void generateChunk(int32_t x, int32_t y, int32_t z){
	
	Chunk_t *handle = chunkMap_get(chunkMap, x, y, z);
	
	handle->x = x;
	handle->y = y;
	handle->z = z;
	srand(gseed); //regenerate random values
	
	chunkFunction(handle);
}

void *waitingRoom(void *arg){
	
	while(programRunning){
		
		pthread_mutex_lock(&jobMutex);
		if(jobQueue){
			//get first job and update next job
			Job_t *myJob = jobQueue;
			jobQueue = jobQueue->nextJob;
			int32_t tmpX = myJob->x;
			int32_t tmpY = myJob->y;
			int32_t tmpZ = myJob->z;
			free(myJob);
			myJob = NULL;
			pthread_mutex_unlock(&jobMutex);
			
			//generate raw chunk data
			generateChunk(tmpX, tmpY, tmpZ);

			//TODO: binary meshing instead of instancing blocks
			
			//update_shadowVBO and draw other one when done
			update_shadowVBO = 1;
		}else{
			pthread_mutex_unlock(&jobMutex);
		}
	}

	return 0;
}


void setUpThreads(){
	
	programRunning = 1;
	
	chunkMap = chunkMap_init(RENDERSPAN);

	pthread_mutex_init(&jobMutex, NULL);
	
	for(uint8_t id = 0; id < CHUNK_THREADS; id++){
		pthread_create(&chunkQueue[id], NULL, &waitingRoom, NULL);
	}
}

//clear threads
void clearThreads(){
	programRunning = 0;//make threads end waiting
	for(uint8_t id = 0; id < CHUNK_THREADS; id++){
		pthread_join(chunkQueue[id], NULL);
	}
}

void addNewChunkJobs(int32_t lastX, int32_t lastY, int32_t lastZ, int32_t currX, int32_t currY, int32_t currZ){
	//TODO: make more efficient
	for(uint32_t z = currZ-RENDERDISTANCE; z <= currZ+RENDERDISTANCE; z++){
		for(uint32_t y = currY-RENDERDISTANCE; y <= currY+RENDERDISTANCE; y++){
			for(uint32_t x = currX-RENDERDISTANCE; x <= currX+RENDERDISTANCE; x++){
				addJob(x, y, z);
			}
		}
	}
}

uint8_t inRenderRegion(Chunk_t *chunk){
	return (
		abs(chunk->x - currChunk.x) <= RENDERDISTANCE &&
		abs(chunk->y - currChunk.y) <= RENDERDISTANCE &&
		abs(chunk->z - currChunk.z) <= RENDERDISTANCE
	);
}
