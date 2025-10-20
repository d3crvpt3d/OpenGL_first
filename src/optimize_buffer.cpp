#include "optimize_buffer.h"
#include "chunkGeneration.h"
#include "chunkMap.h"
#include <array>
#include <cstdint>
#include <vector>

#define TRANSPARENT_MASK 0x80 //uint16_t

std::array<std::vector<QuadGPU_t>, 6> gen_optimized_buffer(ChunkMap &map, int32_t x, int32_t y, int32_t z){

	std::array<std::vector<QuadGPU_t>, 6> out_data;

	//TODO: greedy meshing
	//bool visited [6][64][64][64] = {false}; //face,z,y,x

	//TODO: maybe insert caching
	//for neighbor chunks here

	Chunk_t thisChunk = map.at(x, y, z);

	for(uint8_t z = 0; z < 64; z++){
		for(uint8_t y = 0; y < 64; y++){
			for(uint8_t x = 0; x < 64; x++){

				uint16_t block = thisChunk.blocks[z][y][x];
				bool side_visible[6] = {false};

				//early skip if air
				if(block == 0){
					continue;
				}

				//vector from player pos to chunk
				int32_t vecX = x - currChunk.x;
				int32_t vecY = y - currChunk.y;
				int32_t vecZ = z - currChunk.z;

				//cull x
				if(vecX > 1){

					if(x == 0){
						side_visible[0] = !map.getBlockAtWorldPos(
								thisChunk.x+x-1,
								thisChunk.y+y,
								thisChunk.z+z);
					}else{
						side_visible[0] = !thisChunk.blocks[z][y][x-1];
					}
				
				}else if(vecX < -1){

					if(x == 63){
						side_visible[1] = !map.getBlockAtWorldPos(
								thisChunk.x+x+1,
								thisChunk.y+y,
								thisChunk.z+z);
					}else{
						side_visible[1] = !thisChunk.blocks[z][y][x+1];
					}

				}else{

					if(x == 0){
						side_visible[0] = !map.getBlockAtWorldPos(
								thisChunk.x+x-1,
								thisChunk.y+y,
								thisChunk.z+z);
					}else{
						side_visible[0] = !thisChunk.blocks[z][y][x-1];
					}

					if(x == 63){
						side_visible[1] = !map.getBlockAtWorldPos(
								thisChunk.x+x+1,
								thisChunk.y+y,
								thisChunk.z+z);
					}else{
						side_visible[1] = !thisChunk.blocks[z][y][x+1];
					}

				}

				//cull y
				if(vecY > 1){

					if(y == 0){
						side_visible[2] = !map.getBlockAtWorldPos(
								thisChunk.x+x,
								thisChunk.y+y-1,
								thisChunk.z+z);
					}else{
						side_visible[2] = !thisChunk.blocks[z][y-1][x];
					}
				
				}else if(vecX < -1){

					if(x == 63){
						side_visible[3] = !map.getBlockAtWorldPos(
								thisChunk.x+x,
								thisChunk.y+y+1,
								thisChunk.z+z);
					}else{
						side_visible[3] = !thisChunk.blocks[z][y+1][x];
					}

				}else{

					if(x == 0){
						side_visible[2] = !map.getBlockAtWorldPos(
								thisChunk.x+x,
								thisChunk.y+y-1,
								thisChunk.z+z);
					}else{
						side_visible[2] = !thisChunk.blocks[z][y-1][x];
					}

					if(x == 63){
						side_visible[3] = !map.getBlockAtWorldPos(
								thisChunk.x+x,
								thisChunk.y+y+1,
								thisChunk.z+z);
					}else{
						side_visible[3] = !thisChunk.blocks[z][y+1][x];
					}

				}

				//cull z
				if(vecX > 1){

					if(x == 0){
						side_visible[4] = !map.getBlockAtWorldPos(
								thisChunk.x+x,
								thisChunk.y+y,
								thisChunk.z+z-1);
					}else{
						side_visible[4] = !thisChunk.blocks[z-1][y][x];
					}
				
				}else if(vecX < -1){

					if(x == 63){
						side_visible[5] = !map.getBlockAtWorldPos(
								thisChunk.x+x,
								thisChunk.y+y,
								thisChunk.z+z+1);
					}else{
						side_visible[5] = !thisChunk.blocks[z+1][y][x];
					}

				}else{

					if(x == 0){
						side_visible[4] = !map.getBlockAtWorldPos(
								thisChunk.x+x,
								thisChunk.y+y,
								thisChunk.z+z-1);
					}else{
						side_visible[4] = !thisChunk.blocks[z-1][y][x];
					}

					if(x == 63){
						side_visible[5] = !map.getBlockAtWorldPos(
								thisChunk.x+x,
								thisChunk.y+y,
								thisChunk.z+z+1);
					}else{
						side_visible[5] = !thisChunk.blocks[z+1][y][x];
					}

				}


				//push each visible face
				for(uint8_t side = 0; side < 6; side++){

					if(side_visible[side]){

						QuadGPU_t tmp;
						
						tmp.size[0] = 1;
						tmp.size[1] = 1;
						tmp.type = block;
						tmp.xyz[0] = x;
						tmp.xyz[1] = y;
						tmp.xyz[2] = z;

						out_data[side].push_back(tmp);
					}

				}

			}
		}
	}

	return out_data;
}
