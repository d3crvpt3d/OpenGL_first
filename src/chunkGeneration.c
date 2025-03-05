#include "chunkGeneration.h"

#include <stdio.h>

#define CHUNKDATA E:\Code\Projects\OpenGL\opengl_glfw_1\include\main.h

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

//generate chunk on chunk coord [pos]
void generateChunk(int32_t x, int32_t y, int32_t z){
	
	Chunk_t *handle = chunkMap_get(chunkMap, x, y, z);
	
	if(handle->initialized){
		
		//skip if already generated
		if(inRenderRegion(handle)){
			return;
		}
		
		//save/overwrite file if chunk exists and was modified
		if(handle->modified){
			char *template = "chunkData/________________________.chunk";
			char nameX[8] = {'_'};
			char nameY[8] = {'_'};
			char nameZ[8] = {'_'};
			itoa(handle->x, nameX, 16);
			itoa(handle->y, nameY, 16);
			itoa(handle->z, nameZ, 16);
			memcpy(&template[0]+sizeof("chunkData/")-1,  nameX, 8);
			memcpy(&template[8]+sizeof("chunkData/")-1,  nameY, 8);
			memcpy(&template[16]+sizeof("chunkData/")-1, nameZ, 8);
			FILE *fptr;
			fptr = fopen(template, "w");
			fwrite(handle->blocks, sizeof(handle->blocks), 1, fptr); //write contents of buffer
			fclose(fptr);
		}
	}
	
	//fill chunk memory with new generated values or load from file
	{
		char template[41] = "chunkData/________________________.chunk\0";
		char nameX[8] = {'_'};
		char nameY[8] = {'_'};
		char nameZ[8] = {'_'};
		itoa(x, nameX, 16);
		itoa(y, nameY, 16);
		itoa(z, nameZ, 16);
		memcpy((&template[0])+sizeof("chunkData/")-1, &nameX[0], 8);
		memcpy((&template[8])+sizeof("chunkData/")-1, &nameY[0], 8);
		memcpy((&template[16])+sizeof("chunkData/")-1, &nameZ[0], 8);
		FILE *fptr;
		fptr = fopen(template, "rb");

		if(fptr){
			fread(handle->blocks, sizeof(handle->blocks), 1, fptr);//read contentes into buffer
			fclose(fptr);
			handle->x = x;
			handle->y = y;
			handle->z = z;
			return;
		}
	}
	
	handle->x = x;
	handle->y = y;
	handle->z = z;
	srand(gseed); //regenerate random values
	
	memset(handle->blocks, rand() & 0xFFFF, 14); //DEBUG
}

void *waitingRoom(void *args){
	
	uint8_t id = (uint8_t) args;
	
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
			
			//binary meshing
			uint64_t *binaryData = createBinaryMesh(tmpX, tmpY, tmpZ);
			
			//greedy meshing
			
			//swap active buffer
			
		}else{
			pthread_mutex_unlock(&jobMutex);
		}
	}
}


void setUpThreads(){
	
	programRunning = 1;
	
	chunkMap = chunkMap_init(RENDERSPAN);

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