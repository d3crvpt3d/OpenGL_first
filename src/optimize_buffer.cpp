#include "optimize_buffer.h"
#include "chunkMap.h"
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <vector>

#define TRANSPARENT_MASK 0x80 //uint16_t
#define LOD_BIAS 0

bool transparent(uint16_t id){
	return !id || (id & TRANSPARENT_MASK);
}

//log2 of unsigned integer
uint32_t log2u(uint32_t num){

	if(num == 0){
		return 1;
	}

	return 31 - __builtin_clz(num);
}

//returns lod level based on euclidean distance
//its linear but only in steps of 2^n | n in N
uint32_t lod_level(int32_t x, int32_t y, int32_t z){

	uint32_t distance = sqrt(x*x+y*y+z*z);

	return 1 << (log2u(distance) - 1);
}

std::array<std::vector<QuadGPU_t>, 6> gen_optimized_buffer(ChunkMap &map, int32_t chunkX, int32_t chunkY, int32_t chunkZ){

	const int32_t worldChunkX = (chunkX << 6);
	const int32_t worldChunkY = (chunkY << 6);
	const int32_t worldChunkZ = (chunkZ << 6);

	std::array<std::vector<QuadGPU_t>, 6> out_data;

	//TODO: greedy meshing
	//bool visited [6][64][64][64] = {false}; //face,z,y,x

	Chunk_t *thisChunk = map.at(chunkX, chunkY, chunkZ);

	//shouldnt be possible
	if(thisChunk == nullptr){
		fprintf(stderr,"recieved nullptr from chunkmap\n");
		return out_data;
	}


	//calculate lod level of this chunk
	//used every num blocks
	uint32_t lod_num = lod_level(chunkX, chunkY, chunkZ);


	for(uint8_t z = 0; z < 64; z+= lod_num){
		const int32_t worldZ = worldChunkZ + z;
		for(uint8_t y = 0; y < 64; y+= lod_num){
			const int32_t worldY = worldChunkY + y;
			for(uint8_t x = 0; x < 64; x+= lod_num){
				const int32_t worldX = worldChunkX + x;

				//get actual block
				uint16_t block = thisChunk->blocks[z][y][x];

				//early skip if air
				if(block == 0){
					continue;
				}

				bool side_visible[6] = {false};

				//loop through all skipped blocks
				//and only dont draw if every
				//TODO: test all blocks in neighboring faces instead of this trash implementation
				for(uint8_t partX = 0; partX < lod_num; partX++){
					for(uint8_t partY = 0; partY < lod_num; partY++){
						for(uint8_t partZ = 0; partZ < lod_num; partZ++){

							//check each direction
							side_visible[0] |= (x == 0) ?
								transparent(map.getBlockAtWorldPos(worldX+partX - 1, worldY+partY, worldZ+partZ))
								: transparent(thisChunk->blocks[z+partZ][y+partY][x+partX-1]);

							side_visible[1] |= (x == 63) ?
								transparent(map.getBlockAtWorldPos(worldX+partX + 1, worldY+partY, worldZ+partZ))
								: transparent(thisChunk->blocks[z+partZ][y+partY][x+partX+1]);

							side_visible[2] |= (y == 0) ?
								transparent(map.getBlockAtWorldPos(worldX+partX, worldY+partY - 1, worldZ+partZ))
								: transparent(thisChunk->blocks[z+partZ][y+partY-1][x+partX]);

							side_visible[3] |= (y == 63) ?
								transparent(map.getBlockAtWorldPos(worldX+partX, worldY+partY + 1, worldZ+partZ))
								: transparent(thisChunk->blocks[z+partZ][y+partY+1][x+partX]);

							side_visible[4] |= (z == 0) ?
								transparent(map.getBlockAtWorldPos(worldX+partX, worldY+partY, worldZ+partZ - 1))
								: transparent(thisChunk->blocks[z+partZ-1][y+partY][x+partX]);

							side_visible[5] |= (z == 63) ?
								transparent(map.getBlockAtWorldPos(worldX+partX, worldY+partY, worldZ+partZ + 1))
								: transparent(thisChunk->blocks[z+partZ+1][y+partY][x+partX]);

						}
					}
				}


				//push each visible face
				for(uint8_t side = 0; side < 6; side++){

					if(side_visible[side]){
						QuadGPU_t tmp;

						tmp.size[0] = 1;
						tmp.size[1] = 1;
						tmp.type = block;
						tmp.xyz[0] = worldX;
						tmp.xyz[1] = worldY;
						tmp.xyz[2] = worldZ;

						out_data[side].push_back(tmp);
					}

				}

			}
		}
	}

	return out_data;
}
