#include "chunkGeneration.h"
#include "optimize_buffer.h"
#include "chunkMap.h"
#include "bufferMap.h"
#include "perlinGeneration.hpp"

#include <array>
#include <atomic>
#include <condition_variable>
#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <queue>
#include <sys/types.h>
#include <thread>
#include <stdio.h>
#include <vector>

#define DEBUG_MODE_CHUNK_GEN 0

uint8_t programRunning = 1;
vec3i_t currChunk;
ChunkMap *chunkMap = nullptr;

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

std::atomic<uint32_t> chunkDone{0};
std::atomic<uint32_t> chunksTodo{0};

std::vector<std::thread> threadVec;

std::condition_variable cv;
std::condition_variable updateThreadCV;

std::queue<vec3i_t> jobQueue;

ssize_t face_offset[2][6] = {0}; //offset of each face inside vram buffer

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


void generationWorker(){

	//conditional variable wait
	while(programRunning){

		//lock jobQueue
		std::unique_lock<std::mutex> uqlock(jobMutex);

		//give the mutex away and sleep until jobQueue is not empty
		cv.wait(uqlock, []{return !jobQueue.empty() || !programRunning;});

		if(!programRunning){
			uqlock.unlock();
			cv.notify_one();
			break;
		}

		//get first job and update next job
		vec3i_t currChunkCoord = jobQueue.front();
		int32_t x = currChunkCoord.x;
		int32_t y = currChunkCoord.y;
		int32_t z = currChunkCoord.z;
		jobQueue.pop();

		//save base chunk
		vec3i_t myBaseChunk;
		myBaseChunk.x = currChunk.x;
		myBaseChunk.y = currChunk.y;
		myBaseChunk.z = currChunk.z;

		//unlock jobQueue
		uqlock.unlock();
		cv.notify_one();

		//generate raw chunk data
		generateChunk(x, y, z);

		chunkDone.fetch_add(1, std::memory_order_relaxed);

		//notify chunk to check if now all data generated
		updateThreadCV.notify_one();

	}
}

bool newChunk(){
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
	
	static BufferMap *bufferCache = new BufferMap();

	while(programRunning){

		//new chunk
		if(newChunk()){

			lastChunk.x = currChunk.x;
			lastChunk.y = currChunk.y;
			lastChunk.z = currChunk.z;

			chunkDone.store(0, std::memory_order_release);

			vec3i_t localChunk = {currChunk.x, currChunk.y, currChunk.z};

			//clear queue from last chunk
			jobMutex.lock();
			
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

			chunksTodo.store(localtodo, std::memory_order_relaxed);

			jobMutex.unlock();
			cv.notify_all();

		}else if(chunkDone.load(std::memory_order_relaxed) == chunksTodo.load(std::memory_order_relaxed)
				&& chunksTodo.load(std::memory_order_relaxed) > 0){

			//chunk generation done

			std::array<std::vector<QuadGPU_t>, 6> optimized_buffer_data;

			//optimize each chunk
			int32_t cx = currChunk.x;
			int32_t cy = currChunk.y;
			int32_t cz = currChunk.z;
			for(int32_t z = cz-RENDERDISTANCE; z <= cz+RENDERDISTANCE; z++){
				for(int32_t y = cy-RENDERDISTANCE; y <= cy+RENDERDISTANCE; y++){
					for(int32_t x = cx-RENDERDISTANCE; x <= cx+RENDERDISTANCE; x++){

						//TODO: use lod buffer map for caching
						BufferCache_t *cache = bufferCache->at(x, y, z);

						//check if already optimized
					//	if(	!cache->initialized ||
					//		cache->x != x ||
					//		cache->y != y ||
					//		cache->z != z){

							//not in cache, so generate optimized data
							cache->data = gen_optimized_buffer(*chunkMap,
									x, y, z,
									currChunk.x,
									currChunk.y,
									currChunk.z);
							cache->initialized = true;
							cache->x = x;
							cache->y = y;
							cache->z = z;

					//	}

						//insert data into each side buffer
						for(int side = 0; side < 6; side++){
							optimized_buffer_data[side].insert(
									optimized_buffer_data[side].end(),
									cache->data[side].begin(),
									cache->data[side].end());
						}

					}
				}
			}
			//optimized_buffer_data now has all instance data of all sides

			//realistic max size ~100MB
			ssize_t max_size = sizeof(QuadGPU_t) * CHUNKS * (BLOCKS_PER_CHUNK / 8);

			//get not used buffers
			int write_buf = 1 - current_buffer.load(std::memory_order_relaxed);
			QuadGPU_t *instance_data = (QuadGPU_t*) mapped_regions[write_buf];

			//get total size in bytes
			//+put size/offset information to offset
			//them memcpy data to mapped buffer
			ssize_t copy_size = 0;
			for(int face = 0; face < 6; face++){
				copy_size += sizeof(QuadGPU_t) * optimized_buffer_data[face].size();

				if(face == 0){
					face_offset[write_buf][face] = 0;
				}else{
					face_offset[write_buf][face] = sizeof(QuadGPU_t) * optimized_buffer_data[face-1].size()
						 + face_offset[write_buf][face-1];
				}

				//check if memcpy would work
				if(copy_size > max_size){
					fprintf(stderr, "Buffer overflow prevented! %zu > %zu\n", copy_size, max_size);
				}

				//set size of new data
				instance_count_perBuffer[write_buf][face].store(optimized_buffer_data[face].size(),
						std::memory_order_release);

				//copy optimized instance data to VRAM buffer
				memcpy((QuadGPU_t*) ((uint64_t) instance_data + (uint64_t) face_offset[write_buf][face]),
						optimized_buffer_data[face].data(),
						sizeof(QuadGPU_t) * optimized_buffer_data[face].size());
			}

			//swap vram buffer pointer,
			//now that all data is uploaded
			current_buffer.store(write_buf, std::memory_order_release);

			chunksTodo.store(0, std::memory_order_relaxed);
		}else{
			//currently nothing to do
			std::unique_lock updateThreaduq(updateThreadMutex);

			updateThreadCV.wait(updateThreaduq,
					[]{
					return newChunk() || !programRunning ||
					(chunkDone.load(std::memory_order_relaxed) == chunksTodo.load(std::memory_order_relaxed)
					 && chunksTodo.load(std::memory_order_relaxed) > 0);
					});

		}
	}
}

//create worker threads and one vramUpdateThread
void setUpThreads(){
	
	chunkMap = new ChunkMap();

	programRunning = 1;
	
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
	updateThreadCV.notify_one();

	while(!threadVec.empty()){
		threadVec.back().join();
		threadVec.pop_back();
	}
}
