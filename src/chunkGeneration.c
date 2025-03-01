#include "chunkGeneration.h"

uint8_t programRunning = 1;
uint8_t activeChunks[RENDERSPAN][RENDERSPAN][RENDERSPAN] = {0};

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

pthread_t chunkQueue[CHUNK_THREADS];



//linked list
Job_t *jobQueue = NULL;
Job_t *lastJob = NULL;

pthread_mutex_t jobMutex;

void addJob(int32_t x, int32_t y, int32_t z){
	pthread_mutex_lock(jobMutex);
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
	pthread_mutex_unlock(jobMutex);
}

void *waitingRoom(void *args){
	
	uint8_t id = (uint8_t) args;

	while(programRunning){
	
		pthread_mutex_lock(&jobMutex);
		if(jobQueue){
			//get first job and update next job
			Job_t *myJob = jobQueue;
			vec3i_t myChunkPos = {myJob->x, myJob->y, myJob->z};
			jobQueue = jobQueue->nextJob;
			free(myJob);
			myJob = NULL;
			pthread_mutex_unlock(&jobMutex);

			//generate raw chunk data
			uint16_t chunkMem[CHUNK_WDH][CHUNK_WDH][CHUNK_WDH] = {0};
			generateChunk(&myChunkPos, &chunkMem);

			//binary meshing

			//greedy meshing

			//notify main thread to upload memory region

		}else{
			pthread_mutex_unlock(&jobMutex);
		}
	}
}


void setUpThreads(){
	
	programRunning = 1;

	pthread_mutex_init(&jobMutex, NULL);

	for(uint8_t id = 0; id < CHUNK_THREADS; id++){
		pthread_create(&chunkQueue[id], NULL, &waitingRoom, (void *) id);
	}
}

//clear threads
void clearThreads(){
	programRunning = 0;//make threads end waiting
	for(uint8_t id = 0; id < CHUNK_THREADS; id++){
		pthread_join(chunkQueue[id], NULL);
	}
}

//generate chunk on chunk coord [pos]
void generateChunk(vec3i_t *chunk_coord, uint16_t *chunkMem){
	//already generated
	if(activeChunks[chunk_coord->x][chunk_coord->y][chunk_coord->z]){
		return;
	}
	activeChunks[chunk_coord->x][chunk_coord->y][chunk_coord->z] = 1;
	srand(gseed); //regenerate random values

	memset(chunkMem, 1, 14); //DEBUG //TODO:
}