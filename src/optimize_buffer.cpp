#include "optimize_buffer.h"
#include "chunkMap.h"
#include <array>
#include <cstdint>
#include <cstdio>
#include <vector>

#define TRANSPARENT_MASK 0x80 //uint16_t

bool transparent(uint16_t id){
	return !id || (id & TRANSPARENT_MASK);
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

	for(uint8_t z = 0; z < 64; z++){
		const int32_t worldZ = worldChunkZ + z;
		for(uint8_t y = 0; y < 64; y++){
			const int32_t worldY = worldChunkY + y;
			for(uint8_t x = 0; x < 64; x++){
				const int32_t worldX = worldChunkX + x;

				//get actual block
				uint16_t block = thisChunk->blocks[z][y][x];

				//early skip if air
				if(block == 0){
					continue;
				}

				bool side_visible[6] = {false};

				//check each direction
				side_visible[0] = (x == 0) ?
					transparent(map.getBlockAtWorldPos(worldX - 1, worldY, worldZ))
					: transparent(thisChunk->blocks[z][y][x-1]);

				side_visible[1] = (x == 63) ?
					transparent(map.getBlockAtWorldPos(worldX + 1, worldY, worldZ))
					: transparent(thisChunk->blocks[z][y][x+1]);

				side_visible[2] = (y == 0) ?
					transparent(map.getBlockAtWorldPos(worldX, worldY - 1, worldZ))
					: transparent(thisChunk->blocks[z][y-1][x]);

				side_visible[3] = (y == 63) ?
					transparent(map.getBlockAtWorldPos(worldX, worldY + 1, worldZ))
					: transparent(thisChunk->blocks[z][y+1][x]);

				side_visible[4] = (z == 0) ?
					transparent(map.getBlockAtWorldPos(worldX, worldY, worldZ - 1))
					: transparent(thisChunk->blocks[z-1][y][x]);

				side_visible[5] = (z == 63) ?
					transparent(map.getBlockAtWorldPos(worldX, worldY, worldZ + 1))
					: transparent(thisChunk->blocks[z+1][y][x]);

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
