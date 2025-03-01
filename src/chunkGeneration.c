#include "chunkGeneration.h"

uint8_t programRunning = 1;

Node_t *root = NULL;

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

void addJob(int16_t x, int16_t y, int16_t z){
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

//generate chunk on chunk coord [pos]
void generateChunk(vec3i_t *chunk_coord){
	//skip if already generated
	if(chunkTree_exists(root, chunk_coord->x, chunk_coord->y, chunk_coord->z)){
		return;
	}

	//create new chunk memory in tree
	Chunk_t *newChunk = (Chunk_t *) malloc(sizeof(Chunk_t));
	newChunk->activeBuffer = 0;
	memset(newChunk->blocks, 0, sizeof(newChunk->blocks));
	newChunk->x = chunk_coord->x;
	newChunk->y = chunk_coord->y;
	newChunk->z = chunk_coord->z;
	chunkTree_insert(root, newChunk);

	//fill chunk memory with generated values

	srand(gseed); //regenerate random values

	memset(newChunk, 1, 14); //DEBUG //TODO:
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
			generateChunk(&myChunkPos);

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

void removeChunk(int16_t x, int16_t y, int16_t z){

}

void addNewChunkJobs(int16_t lastX, int16_t lastY, int16_t lastZ, int16_t currX, int16_t currY, int16_t currZ){
	//TODO: make more efficient
	for(uint16_t z = currZ-RENDERDISTANCE; z <= currZ+RENDERDISTANCE; z++){
		for(uint16_t y = currY-RENDERDISTANCE; y <= currY+RENDERDISTANCE; y++){
			for(uint16_t x = currX-RENDERDISTANCE; x <= currX+RENDERDISTANCE; x++){
				addJob(x, y, z);
			}
		}
	}
}


void removeChunks(int16_t lastX, int16_t lastY, int16_t lastZ, int16_t currX, int16_t currY, int16_t currZ){
	//TODO: treewalk and pop every that is not in render range
}