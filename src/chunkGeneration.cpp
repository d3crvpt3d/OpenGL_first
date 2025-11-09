#include "chunkGeneration.h"
#include "bufferMap.h"
#include "optimize_buffer.h"
#include "chunkMap.h"
#include "perlinGeneration.hpp"

#include <barrier> //c++20
#include <atomic>
#include <condition_variable>
#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <mutex>
#include <queue>
#include <sys/types.h>
#include <thread>
#include <vector>

#define DEBUG_MODE_CHUNK_GEN 0

uint8_t programRunning = 1;
vec3i_t currChunk;
ChunkMap *chunkMap = nullptr;

//queue for the main thread to upload to VBO
std::queue<BufferCache_t> toUploadQueue;
BufferMap bufferMap = BufferMap();
std::mutex toUploadQueue_mutex;
//generate Spawn location by lastChunk != currChunk
vec3i_t lastChunk;

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

std::mutex jobMutex;
std::mutex updateThreadMutex;
std::mutex currChunk_mutex;

std::vector<std::thread> threadVec;

std::condition_variable cv;
std::condition_variable updateThreadCV;

std::atomic<uint32_t> chunksOptimized{0};
std::atomic<uint32_t> chunksGenerated{0};

std::queue<vec3i_t> jobQueue;

void addJob(int32_t x, int32_t y, int32_t z){

	jobQueue.push({x, y, z});

}


//generate chunk on chunk coord [pos]
void generateChunk(int32_t x, int32_t y, int32_t z){
	
	Chunk_t *handle = chunkMap->at(x, y, z);

	handle->x = x;
	handle->y = y;
	handle->z = z;

	handle->initialized = 1;
	
	generate_chunk_with_caves(*chunkMap, x, y, z);

}

typedef struct {
	vec3i_t xyz;
	vec3i_t chunk;
} optimizeElement_t;

std::queue<optimizeElement_t> optimizeQueue;
std::mutex optimizeQueue_mutex;

void optimizeTask(int x, int y, int z, int cx, int cy, int cz){

	BufferCache_t tmp = std::move(
			gen_optimized_buffer(*chunkMap,
				x, y, z,
				cx,
				cy,
				cz)
			);
	bool empty = true;
	for(uint8_t i = 0; i < 6; i++){
		empty = empty && tmp.size[i] == 0;
	}
	if(empty){
		return;
	}

	toUploadQueue_mutex.lock();

	toUploadQueue.push(std::move(tmp));

	toUploadQueue_mutex.unlock();
}

std::barrier sync_point(std::thread::hardware_concurrency() - 1);

void generationWorker(){
	
	while(programRunning){

		//wait until jobQueue not empty and
		//have mutex
		{
			std::unique_lock<std::mutex> lock(jobMutex);
			cv.wait(lock, []{return !programRunning || !jobQueue.empty();});
		}
		cv.notify_all();

		if(!programRunning){
			break;
		}

		//generate chunk data
		while(true){

			if(!programRunning){
				return;
			}

			//lock jobQueue
			jobMutex.lock();

			if(jobQueue.empty()){
				break;
			}

			//get first job and update next job
			vec3i_t currChunkCoord = jobQueue.front();
			jobQueue.pop();
			jobMutex.unlock();
			cv.notify_all();

			int32_t x = currChunkCoord.x;
			int32_t y = currChunkCoord.y;
			int32_t z = currChunkCoord.z;

			//generate raw chunk data
			generateChunk(x, y, z);
			chunksGenerated.fetch_add(1, std::memory_order_relaxed);
			updateThreadCV.notify_all();
		}

		//wait until all threads are
		//done with chunk gen
		sync_point.arrive_and_wait();

		//optimize chunk data
		//TODO: optimize only adjacent chunks
		while(true){

			std::unique_lock<std::mutex> lock(optimizeQueue_mutex);
			cv.wait(lock, []{
					return !optimizeQueue.empty() || !programRunning;});
			
			if(!programRunning){
				return;
			}

			//chunk optimize done
			if(optimizeQueue.empty()){
				break;
			}

			optimizeElement_t a = optimizeQueue.front();
			optimizeQueue.pop();

			lock.unlock();
			cv.notify_all();

			optimizeTask(a.xyz.x, a.xyz.y, a.xyz.z, a.chunk.x, a.chunk.y, a.chunk.z);
			chunksOptimized.fetch_add(1, std::memory_order_relaxed);
			updateThreadCV.notify_all();
		}

	}
}

bool newChunk(){
	std::lock_guard<std::mutex> lock(currChunk_mutex);
	return	currChunk.x != lastChunk.x ||
			currChunk.y != lastChunk.y ||
			currChunk.z != lastChunk.z;
}

//check if new chunk and when all chunks
//for this new chunk are done optimize
//and upload them
void updateVramWorker(){

	lastChunk.x = -2;
	lastChunk.y = -2;
	lastChunk.z = -2;
	
	while(programRunning){

		//new chunk
		if(newChunk()){

			vec3i_t localChunk;
			{
				std::lock_guard<std::mutex> lock(currChunk_mutex);
				lastChunk.x = currChunk.x;
				lastChunk.y = currChunk.y;
				lastChunk.z = currChunk.z;

				localChunk = currChunk;
			}

			jobMutex.lock();
			chunksGenerated.store(0, std::memory_order_relaxed);
			chunksOptimized.store(0, std::memory_order_relaxed);
			//should not be possible
			while(!jobQueue.empty()){
				jobQueue.pop();
			}

			//add jobs to jobQueue for each chunk
			//not initialized or wrong coord
			//in chunk map
			uint32_t localtodo = 0;
			for(int z = localChunk.z-RENDERDISTANCE; z <= localChunk.z+RENDERDISTANCE; z++){
				for(int y = localChunk.y-RENDERDISTANCE; y <= localChunk.y+RENDERDISTANCE; y++){
					for(int x = localChunk.x-RENDERDISTANCE; x <= localChunk.x+RENDERDISTANCE; x++){

						Chunk_t *handle = chunkMap->at(x, y, z);

						//check chunk validity
						if( handle->x != x ||
							handle->y != y ||
							handle->z != z ||
							!handle->initialized){

							//rewrite in modular chunkmap
							addJob(x, y, z);
							localtodo += 1;

						}

					}
				}
			}
			//get optimize mutex before letting other threads
			//free from jobMutex
			optimizeQueue_mutex.lock();

			jobMutex.unlock();
			cv.notify_all();

			//wait for generation done
			{
				std::unique_lock<std::mutex> lock(jobMutex);
				updateThreadCV.wait(lock, [localtodo]{
						return localtodo >= chunksGenerated.load(std::memory_order_relaxed);});
			}

			//---chunk generation done---

			//optimize each chunk
			int32_t cx = localChunk.x;
			int32_t cy = localChunk.y;
			int32_t cz = localChunk.z;
			for(int32_t z = cz-RENDERDISTANCE; z <= cz+RENDERDISTANCE; z++){
				for(int32_t y = cy-RENDERDISTANCE; y <= cy+RENDERDISTANCE; y++){
					for(int32_t x = cx-RENDERDISTANCE; x <= cx+RENDERDISTANCE; x++){

						//tell workers to optimize this chunk
						optimizeQueue.push({{x, y, z}, {cx, cy, cz}});

					}
				}
			}

			optimizeQueue_mutex.unlock();
			//tell worker threads to start
			cv.notify_all();
		}else{
			//currently nothing to do
			std::unique_lock updateThreaduq(updateThreadMutex);

			updateThreadCV.wait(updateThreaduq,
					[]{return newChunk() || !programRunning;});
		}
	}
}

//create worker threads and one vramUpdateThread
void setUpThreads(){
	
	chunkMap = new ChunkMap();

	programRunning = 1;
	uint32_t num_threads = std::thread::hardware_concurrency();

	threadVec.reserve(num_threads);

	//chunk generation threads
	for(uint32_t id = 0; id < num_threads - 1; id++){
		threadVec.push_back(std::thread(generationWorker));
	}

	//updateVramWorker
	threadVec.push_back(std::thread(updateVramWorker));

}

//clear threads
void clearThreads(){
	programRunning = 0;//make threads end waiting
	cv.notify_all(); //notify if currently waiting
	updateThreadCV.notify_one();

	while(!threadVec.empty()){
		threadVec.back().join();
		threadVec.pop_back();
	}
}
