#include "chunkGeneration.h"

void *generateChunk(void *position){



	return NULL;
}

void generateChunks(int32_t x, int32_t y, int32_t z, Chunk *chunks){
	
	int32_t currChunk[3] = {x/CHUNK_WDH, y/CHUNK_WDH, z/CHUNK_WDH};

	ThreadPool *pool = (ThreadPool *) malloc(sizeof(ThreadPool));

	if(!pool){
		return NULL;
	}

	pool->threads = (pthread_t *) malloc(sizeof(pthread_t) * get_max_threads());
	if(!pool->thread_count){
		free(pool);
		return NULL;
	}
	pool->thread_count = get_max_threads();

	//iterate through all chunks and send (generation/read from memory) jobs to chunks currently not generated
	for(int32_t curr_x = currChunk[0] - RENDERDISTANCE; curr_x < currChunk[0] + RENDERDISTANCE; curr_x++){
		for(int32_t curr_y = currChunk[1] - RENDERDISTANCE; curr_y < currChunk[1] + RENDERDISTANCE; curr_y++){
			for(int32_t curr_z = currChunk[2] - RENDERDISTANCE; curr_z < currChunk[2] + RENDERDISTANCE; curr_z++){

				if(chunks[0].data){
					//TODO:
					if(){

					}
				}

			}
		}
	}

}