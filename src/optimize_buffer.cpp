#include "optimize_buffer.h"
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

				//push/gen block buffer data if visible
				//push if on bounds
				if( (x == 0 || x == 63) ||
					(y == 0 || y == 63) ||
					(z == 0 || z == 63) ||
					(chunk.blocks[z][y][x] && !(
					chunk.blocks[z][y][x-1] && chunk.blocks[z][y][x+1] && //x visible
					chunk.blocks[z][y-1][x] && chunk.blocks[z][y+1][x] && //y visible
					chunk.blocks[z-1][y][x] && chunk.blocks[z+1][y][x]) ) //z visible
					){ 

					BlockGPU_t packed_data = {};

					packed_data.xyz[0] = 64 * chunk.x + x;
					packed_data.xyz[1] = 64 * chunk.y + y;
					packed_data.xyz[2] = 64 * chunk.z + z;
					
					packed_data.type = chunk.blocks[z][y][x]; //block type

					out_data.push_back(packed_data);
				
				}

			}
		}
	}

	chunk.bufferSize = out_data.size(); //give information about buffer size

	return out_data;
}
