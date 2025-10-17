#include "chunkGeneration.h"
#include "optimize_buffer.h"
#include "chunkMap.h"
#include "perlinGeneration.hpp"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <queue>
#include <thread>
#include <stdio.h>
#include <vector>

#define DEBUG_MODE_CHUNK_GEN 0

uint8_t programRunning = 1;
vec3i_t currChunk = {0, 0, 0};
ChunkMap_t *chunkMap = NULL;

//generate Spawn location by lastChunk != currChunk
vec3i_t lastChunk = {-1, -1, -1};

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
std::mutex chunkDoneMutex;

std::atomic<uint32_t> chunkDone{0};
std::atomic<uint32_t> chunksTodo{0};

std::vector<std::thread> threadVec;

std::condition_variable cv;

std::queue<vec3i_t> jobQueue;

void addJob(int32_t x, int32_t y, int32_t z){

	jobQueue.push({x, y, z});

}


//generate chunk on chunk coord [pos]
void generateChunk(int32_t x, int32_t y, int32_t z){
	
	Chunk_t *handle = chunkMap_get(chunkMap, x, y, z);

	handle->x = x;
	handle->y = y;
	handle->z = z;

	handle->initialized = 1;
	
	generate_chunk_with_caves(*handle, x, y, z);

}


void generationWorker(){
	
	//conditional variable wait
	while(programRunning){

		//lock jobQueue
		std::unique_lock<std::mutex> uqlock(jobMutex);

		//give the mutex away and sleep until jobQueue is not empty
		cv.wait(uqlock, []{return !jobQueue.empty() || !programRunning;});

		if(!programRunning){
			break;
		}

		//get first job and update next job
		vec3i_t currChunkCoord = jobQueue.front();
		int32_t x = currChunkCoord.x;
		int32_t y = currChunkCoord.y;
		int32_t z = currChunkCoord.z;
		jobQueue.pop();

		//save base chunk
		vec3i_t myBaseChunk = currChunk;

		//unlock jobQueue
		uqlock.unlock();

		//generate raw chunk data
		generateChunk(x, y, z);

		printf("Generated chunk (%d, %d, %d)\n", x, y, z);//DEBUG

		//increase atomic by 1 if done for right base chunk
		chunkDoneMutex.lock();

		if( myBaseChunk.x == currChunk.x &&
			myBaseChunk.y == currChunk.y &&
			myBaseChunk.z == currChunk.z
			){

			chunkDone.fetch_add(1, std::memory_order_relaxed);
			
		}

		chunkDoneMutex.unlock();
	}
}

//check if new chunk and when all chunks
//for this new chunk are done optimize
//and upload them
void updateVramWorker(){

	printf("[VRAM] Thread started!\n");
	printf("[VRAM] Initial currChunk: (%d, %d, %d)\n", currChunk.x, currChunk.y, currChunk.z);
	printf("[VRAM] Initial lastChunk: (%d, %d, %d)\n", lastChunk.x, lastChunk.y, lastChunk.z);
	fflush(stdout);

	while(programRunning){

		printf("[VRAM] Loop iteration: currChunk=(%d,%d,%d) lastChunk=(%d,%d,%d)\n",
               currChunk.x, currChunk.y, currChunk.z,
               lastChunk.x, lastChunk.y, lastChunk.z);

		if( currChunk.x != lastChunk.x ||
			currChunk.y != lastChunk.y ||
			currChunk.z != lastChunk.z){
			
            printf("[VRAM] ENTERING IF BLOCK!\n");

			lastChunk.x = currChunk.x;
			lastChunk.y = currChunk.y;
			lastChunk.z = currChunk.z;


            printf("[VRAM] reset chunkDone to 0!\n");
			chunkDone.store(0, std::memory_order_release);

            printf("[VRAM] locking jobMutex!\n");
			//clear queue from last chunk
			jobMutex.lock();
			
            printf("[VRAM] empty jobQueue!\n");
			while(!jobQueue.empty()){
				jobQueue.pop();
			}

            printf("[VRAM] Checking which job to add!\n");
			//add jobs to jobQueue for each chunk
			//not initialized or wrong coord
			//in chunk map
			uint32_t localtodo = 0;
			for(int z = currChunk.z-RENDERDISTANCE; z <= currChunk.z+RENDERDISTANCE; z++){
				for(int y = currChunk.y-RENDERDISTANCE; y <= currChunk.y+RENDERDISTANCE; y++){
					for(int x = currChunk.x-RENDERDISTANCE; x <= currChunk.x+RENDERDISTANCE; x++){

						Chunk_t *handle = chunkMap_get(chunkMap, x, y, z);

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

            printf("[VRAM] store chunksTodo to number of jobs!\n");
			chunksTodo.store(localtodo);

			jobMutex.unlock();
			cv.notify_all();

		}else{

			//write directly to not used buffer
			int write_buf = 1 - current_buffer.load(std::memory_order_relaxed);
			BlockGPU_t *instance_data = (BlockGPU_t*) mapped_regions[write_buf];

			std::vector<BlockGPU_t> optimized_buffer_data;

			//push chunk data to non-active blockVBO
			if(chunkDone.load() == chunksTodo.load() && chunksTodo.load() > 0){

				//iterate each chunk in RENDERDISTANCE
				for(int z = currChunk.z-RENDERDISTANCE; z <= currChunk.z+RENDERDISTANCE; z++){
					for(int y = currChunk.y-RENDERDISTANCE; y <= currChunk.y+RENDERDISTANCE; y++){
						for(int x = currChunk.x-RENDERDISTANCE; x <= currChunk.x+RENDERDISTANCE; x++){

							//get RAM data
							Chunk_t *chunk = chunkMap_get(chunkMap, x, y, z);

							//append optimized Data for VRAM
							std::vector<BlockGPU_t> currentblocks = gen_optimized_buffer(*chunk);

							optimized_buffer_data.insert(
									optimized_buffer_data.end(),
									currentblocks.begin(),
									currentblocks.end()) ;

						}
					}
				}

				auto copy_size = optimized_buffer_data.size() * sizeof(BlockGPU_t);
				auto max_size = sizeof(BlockGPU_t) * CHUNKS * BLOCKS_PER_CHUNK;

				if(copy_size > max_size){
					fprintf(stderr, "Buffer overflow prevented! %zu > %zu\n", copy_size, max_size);
				}

				//copy optimized instance data to VRAM buffer
				memcpy(instance_data,
						optimized_buffer_data.data(),
						copy_size);

				printf("Copied %zu blocks to GPU\n", optimized_buffer_data.size());//DEBUG

				//swap currently active buffer
				current_buffer.store(write_buf, std::memory_order_release);
				instance_count_perBuffer[write_buf].store(optimized_buffer_data.size(),
						std::memory_order_release);


				chunksTodo.store(0);
			}else{
				//currently nothing to do
				printf("ChunkDone: %d, ChunksTodo: %d\n",
						chunkDone.load(), chunksTodo.load());
				std::this_thread::sleep_for(std::chrono::microseconds(100));
			}
		}
	}
}

//create worker threads and one vramUpdateThread
void setUpThreads(){
	
	programRunning = 1;

	lastChunk.x = -99;
	lastChunk.y = -99;
	lastChunk.z = -99;
	
	chunkMap = chunkMap_init(RENDERSPAN);

	threadVec.reserve(std::thread::hardware_concurrency());

	//chunk generation threads
	for(uint32_t id = 0; id < std::thread::hardware_concurrency() - 1; id++){
		threadVec.push_back(std::thread(generationWorker));
	}

	//updateVramWorker
	threadVec.push_back(std::thread(updateVramWorker));

}

//clear threads
void clearThreads(){
	programRunning = 0;//make threads end waiting
	cv.notify_all(); //notify if currently waiting

	for(uint32_t id = 0; id < threadVec.size(); id++){
		threadVec.back().join();
		threadVec.pop_back();
	}
}
