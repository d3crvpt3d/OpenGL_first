#include "optimize_buffer.h"
#include "chunkMap.h"
#include <cstdint>
#include <vector>

#define SKIP_OPTIMIZED 0

std::vector<Block_t> gen_optimized_buffer(Chunk_t &chunk){


	std::vector<Block_t> out_data;

	//DEBUG
	if(SKIP_OPTIMIZED){

		for(uint8_t z = 0; z < 64; z++){
			for(uint8_t y = 0; y < 64; y++){
				for(uint8_t x = 0; x < 64; x++){

					Block_t packed_data = 0;
					packed_data |= (x & 63) << 26; // x:6
					packed_data |= (y & 63) << 20; // y:6
					packed_data |= (z & 63) << 14; // z:6
					//packed_data |= ( something & 4 ) << 10; // block metadata:4
					packed_data |= chunk.blocks[z][y][x] & 1023; //block type:10

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

					Block_t packed_data = 0;
					packed_data |= (x & 63) << 26; // x:6
					packed_data |= (y & 63) << 20; // y:6
					packed_data |= (z & 63) << 14; // z:6
					//packed_data |= ( something & 4 ) << 10; // block metadata:4
					packed_data |= chunk.blocks[z][y][x] & 1023; //block type:10

					out_data.push_back(packed_data);
				
				}

			}
		}
	}

	chunk.bufferSize = out_data.size(); //give information about buffer size

	return out_data;
}
