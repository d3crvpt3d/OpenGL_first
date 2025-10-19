#include "optimize_buffer.h"
#include "chunkGeneration.h"
#include "chunkMap.h"
#include <cstdint>
#include <vector>

#define SKIP_OPTIMIZED 0

std::vector<BlockGPU_t> gen_optimized_buffer(Chunk_t &chunk){

	std::vector<BlockGPU_t> out_data;

	//DEBUG
	if(SKIP_OPTIMIZED){

		for(uint8_t z = 0; z < 64; z++){
			for(uint8_t y = 0; y < 64; y++){
				for(uint8_t x = 0; x < 64; x++){

					BlockGPU_t packed_data = {};

					packed_data.xyz[0] = 64 * chunk.x + x;
					packed_data.xyz[1] = 64 * chunk.y + y;
					packed_data.xyz[2] = 64 * chunk.z + z;
					
					packed_data.type = chunk.blocks[z][y][x]; //block type

					out_data.push_back(packed_data);
				}
			}
		}
	
		chunk.bufferSize = out_data.size(); //give information about buffer size

		return out_data;
	}

	for(uint8_t z = 0; z < 64; z++){
		for(uint8_t y = 0; y < 64; y++){
			for(uint8_t x = 0; x < 64; x++){

				int32_t xVec = chunk.x - currChunk.x;
				int32_t yVec = chunk.y - currChunk.y;
				int32_t zVec = chunk.z - currChunk.z;

				//early skip if air block
				if(!chunk.blocks[z][y][x]){
					continue;
				}

				//dont check out of bounds
				bool x0 = x == 0;
				bool x63 = x == 63;
				bool y0 = y == 0;
				bool y63 = y == 63;
				bool z0 = z == 0;
				bool z63 = z == 63;

				//chunk more then 1 left of playerChunk
				if(xVec < -1){
					if(x63 || !chunk.blocks[z][y][x+1]){
						goto pack_and_push;
					}
				//chunk more then 1 right of playerChunk
				}else if(xVec > 1){
					if(x0 || !chunk.blocks[z][y][x-1]){
						goto pack_and_push;
					}
				//check both sides
				}else{
					if( (x0 || x63) || !(chunk.blocks[z][y][x-1] && chunk.blocks[z][y][x+1]) ){
						goto pack_and_push;
					}
				}

				//chunk more then 1 above playerChunk
				if(yVec < -1){
					if(y63 || !chunk.blocks[z][y+1][x]){
						goto pack_and_push;
					}
				//chunk more then 1 below playerChunk
				}else if(yVec > 1){
					if(y0 || !chunk.blocks[z][y-1][x]){
						goto pack_and_push;
					}
				//check both sides
				}else{
					if( (y0 || y63) || !(chunk.blocks[z][y-1][x] && chunk.blocks[z][y+1][x]) ){
						goto pack_and_push;
					}
				}

				//chunk more then 1 in front of playerChunk
				if(zVec < -1){
					if(z63 || !chunk.blocks[z+1][y][x]){
						goto pack_and_push;
					}
				//chunk more then 1 behind of playerChunk
				}else if(zVec > 1){
					if(z0 || !chunk.blocks[z-1][y][x]){
						goto pack_and_push;
					}
				//check both sides
				}else{
					if( (z0 || z63) || !(chunk.blocks[z-1][y][x] && chunk.blocks[z+1][y][x]) ){
						goto pack_and_push;
					}
				}

				//not visible
				continue;

				pack_and_push:
				BlockGPU_t packed_data = {};

				packed_data.xyz[0] = 64 * chunk.x + x;
				packed_data.xyz[1] = 64 * chunk.y + y;
				packed_data.xyz[2] = 64 * chunk.z + z;

				packed_data.type = chunk.blocks[z][y][x]; //block type

				out_data.push_back(packed_data);

			}
		}
	}

	chunk.bufferSize = out_data.size(); //give information about buffer size

	return out_data;
}
