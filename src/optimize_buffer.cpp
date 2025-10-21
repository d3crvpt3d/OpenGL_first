#include "optimize_buffer.h"
#include "chunkGeneration.h"
#include "chunkMap.h"
#include <array>
#include <cstdint>
#include <vector>

#define TRANSPARENT_MASK 0x80 //uint16_t

std::array<std::vector<QuadGPU_t>, 6> gen_optimized_buffer(ChunkMap &map, int32_t chunkX, int32_t chunkY, int32_t chunkZ){

	std::array<std::vector<QuadGPU_t>, 6> out_data;

	//TODO: greedy meshing
	//bool visited [6][64][64][64] = {false}; //face,z,y,x

	//TODO: maybe insert caching
	//for neighbor chunks here

	Chunk_t *thisChunk = map.at(chunkX, chunkY, chunkZ);

	for(uint8_t z = 0; z < 64; z++){
		for(uint8_t y = 0; y < 64; y++){
			for(uint8_t x = 0; x < 64; x++){

				uint16_t block = thisChunk->blocks[z][y][x];
				bool side_visible[6] = {false};

				//early skip if air
				if(block == 0){
					continue;
				}

				//vector from player pos to chunk
				int32_t vecX = chunkX - currChunk.x;
				int32_t vecY = chunkY - currChunk.y;
				int32_t vecZ = chunkZ - currChunk.z;

				//cull x
				if(vecX > 1){

					if(x == 0){
						side_visible[0] = !map.getBlockAtWorldPos(
								chunkX+x-1,
								chunkY+y,
								chunkZ+z);
					}else{
						side_visible[0] = !thisChunk->blocks[z][y][x-1];
					}
				
				}else if(vecX < -1){

					if(x == 63){
						side_visible[1] = !map.getBlockAtWorldPos(
								chunkX+x+1,
								chunkY+y,
								chunkZ+z);
					}else{
						side_visible[1] = !thisChunk->blocks[z][y][x+1];
					}

				}else{

					if(x == 0){
						side_visible[0] = !map.getBlockAtWorldPos(
								chunkX+x-1,
								chunkY+y,
								chunkZ+z);
					}else{
						side_visible[0] = !thisChunk->blocks[z][y][x-1];
					}

					if(x == 63){
						side_visible[1] = !map.getBlockAtWorldPos(
								chunkX+x+1,
								chunkY+y,
								chunkZ+z);
					}else{
						side_visible[1] = !thisChunk->blocks[z][y][x+1];
					}

				}

				//cull y
				if(vecY > 1){

					if(y == 0){
						side_visible[2] = !map.getBlockAtWorldPos(
								chunkX+x,
								chunkY+y-1,
								chunkZ+z);
					}else{
						side_visible[2] = !thisChunk->blocks[z][y-1][x];
					}
				
				}else if(vecY < -1){

					if(y == 63){
						side_visible[3] = !map.getBlockAtWorldPos(
								chunkX+x,
								chunkY+y+1,
								chunkZ+z);
					}else{
						side_visible[3] = !thisChunk->blocks[z][y+1][x];
					}

				}else{

					if(y == 0){
						side_visible[2] = !map.getBlockAtWorldPos(
								chunkX+x,
								chunkY+y-1,
								chunkZ+z);
					}else{
						side_visible[2] = !thisChunk->blocks[z][y-1][x];
					}

					if(y == 63){
						side_visible[3] = !map.getBlockAtWorldPos(
								chunkX+x,
								chunkY+y+1,
								chunkZ+z);
					}else{
						side_visible[3] = !thisChunk->blocks[z][y+1][x];
					}

				}

				//cull z
				if(vecZ > 1){

					if(z == 0){
						side_visible[4] = !map.getBlockAtWorldPos(
								chunkX+x,
								chunkY+y,
								chunkZ+z-1);
					}else{
						side_visible[4] = !thisChunk->blocks[z-1][y][x];
					}
				
				}else if(vecZ < -1){

					if(z == 63){
						side_visible[5] = !map.getBlockAtWorldPos(
								chunkX+x,
								chunkY+y,
								chunkZ+z+1);
					}else{
						side_visible[5] = !thisChunk->blocks[z+1][y][x];
					}

				}else{

					if(z == 0){
						side_visible[4] = !map.getBlockAtWorldPos(
								chunkX+x,
								chunkY+y,
								chunkZ+z-1);
					}else{
						side_visible[4] = !thisChunk->blocks[z-1][y][x];
					}

					if(z == 63){
						side_visible[5] = !map.getBlockAtWorldPos(
								chunkX+x,
								chunkY+y,
								chunkZ+z+1);
					}else{
						side_visible[5] = !thisChunk->blocks[z+1][y][x];
					}

				}


				//push each visible face
				for(uint8_t side = 0; side < 6; side++){

					if(side_visible[side]){

						QuadGPU_t tmp;
						
						tmp.size[0] = 1;
						tmp.size[1] = 1;
						tmp.type = block;
						tmp.xyz[0] = (chunkX << 6) + x;
						tmp.xyz[1] = (chunkY << 6) + y;
						tmp.xyz[2] = (chunkZ << 6) + z;

						out_data[side].push_back(tmp);
					}

				}

			}
		}
	}

	return out_data;
}
