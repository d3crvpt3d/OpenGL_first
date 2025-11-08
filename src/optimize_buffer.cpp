#include "optimize_buffer.h"
#include "chunkMap.h"
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <vector>

#define TRANSPARENT_MASK 0x80 //uint16_t
#define LOD_BIAS -1

bool transparent(uint16_t id){
	return !id || (id & TRANSPARENT_MASK);
}

//log2 of unsigned integer
uint32_t log2u(uint32_t num){

	if(num == 0){
		return 0;
	}

	return 31 - __builtin_clz(num);
}

//returns lod number based on euclidean distance
//its linear but only in steps of 2^n | n in N
//{1,1,2,2,4,4,4,4,8,8,...}
// n = n + LOD_BIAS
uint32_t lod_number(int32_t x, int32_t y, int32_t z){

	uint32_t distance = sqrt(x*x+y*y+z*z);

	uint32_t comp = log2u(distance) + LOD_BIAS;

	//clamp to 1 if "negative"
	return 1 << (comp <= 6 ? comp : 0);
}

//get lod index
//n | x = 2^n
//1=0, 2=1, 4=2, ...
uint32_t lod_index(int32_t x, int32_t y, int32_t z){

	uint32_t distance = sqrt(x*x+y*y+z*z);

	uint32_t comp = log2u(distance) + LOD_BIAS;

	//clamp to 1 if "negative"
	return comp <= 6 ? comp : 0;
}

BufferCache_t gen_optimized_buffer(
		ChunkMap &map,
		int32_t cx,
		int32_t cy,
		int32_t cz,
		int32_t px,
		int32_t py,
		int32_t pz){

	const int32_t worldChunkX = (cx << 6);
	const int32_t worldChunkY = (cy << 6);
	const int32_t worldChunkZ = (cz << 6);

	std::array<std::vector<QuadGPU_t>, 6> out_data;

	//TODO: greedy meshing
	//bool visited [6][64][64][64] = {false}; //face,z,y,x

	Chunk_t *thisChunk = map.at(cx, cy, cz);

	//shouldnt be possible
	if(thisChunk == nullptr){
		fprintf(stderr,"recieved nullptr from chunkmap\n");
		BufferCache_t empty = {0};
		empty.x = cx;
		empty.y = cy;
		empty.z = cz;
		return empty;
	}


	//calculate lod number of this chunk
	uint32_t lod_num = lod_number(cx-px, cy-py, cz-pz);


	//check only every 'lod_num' block and
	//generate quads with size of lod_num
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

				//TODO: if not same chunk, check every of face
				//check each direction
				//-x
				if (x == 0){
					for(uint32_t a = 0; a < lod_num; a++){
						for(uint32_t b = 0; b < lod_num; b++){
							side_visible[0] |= transparent(
									map.getBlockAtWorldPos(worldX-1, worldY+a, worldZ+b));
						}
					}
				}else{
					side_visible[0] = transparent(thisChunk->blocks[z][y][x-lod_num]);
				}
				
				//+x
				if (x+lod_num > 63){
					for(uint32_t a = 0; a < lod_num; a++){
						for(uint32_t b = 0; b < lod_num; b++){
							side_visible[1] |= transparent(
									map.getBlockAtWorldPos(worldX + lod_num, worldY+a, worldZ+b));
						}
					}
				}else{
					side_visible[1] = transparent(thisChunk->blocks[z][y][x+lod_num]);
				}

				//-y
				if (y == 0){
					for(uint32_t a = 0; a < lod_num; a++){
						for(uint32_t b = 0; b < lod_num; b++){
							side_visible[2] |= transparent(
									map.getBlockAtWorldPos(worldX+a, worldY-1, worldZ+b));
						}
					}
				}else{
					side_visible[2] = transparent(thisChunk->blocks[z][y-lod_num][x]);
				}
				
				//+y
				if (y+lod_num > 63){
					for(uint32_t a = 0; a < lod_num; a++){
						for(uint32_t b = 0; b < lod_num; b++){
							side_visible[3] |= transparent(
									map.getBlockAtWorldPos(worldX+a, worldY+lod_num, worldZ+b));
						}
					}
				}else{
					side_visible[3] = transparent(thisChunk->blocks[z][y+lod_num][x]);
				}

				//-z
				if (z == 0){
					for(uint32_t a = 0; a < lod_num; a++){
						for(uint32_t b = 0; b < lod_num; b++){
							side_visible[4] |= transparent(
									map.getBlockAtWorldPos(worldX+a, worldY+b, worldZ-1));
						}
					}
				}else{
					side_visible[4] = transparent(thisChunk->blocks[z-lod_num][y][x]);
				}
				
				//+z
				if (z+lod_num > 63){
					for(uint32_t a = 0; a < lod_num; a++){
						for(uint32_t b = 0; b < lod_num; b++){
							side_visible[5] |= transparent(
									map.getBlockAtWorldPos(worldX+a, worldY+b, worldZ+lod_num));
						}
					}
				}else{
					side_visible[5] = transparent(thisChunk->blocks[z+lod_num][y][x]);
				}


				//push each visible face
				for(uint8_t side = 0; side < 6; side++){

					if(side_visible[side]){
						QuadGPU_t tmp;

						tmp.size[0] = lod_num;
						tmp.size[1] = lod_num;
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
	//end z
	
	//pack into one buffer

	BufferCache_t packed_data = {0};

	packed_data.x = cx;
	packed_data.y = cy;
	packed_data.z = cz;

	//reserve space in packed_data buffer
	uint64_t reserve = 0;
	for(uint32_t side = 0; side < 6; side ++){

		//set offset of face in byte
		packed_data.offset[side] = reserve * sizeof(QuadGPU_t);

		//set size of face
		packed_data.size[side] = out_data.at(side).size();

		//add num elements of side buffer to reserve
		reserve += out_data.at(side).size();
	}
	packed_data.data.reserve(reserve);

	//pack data
	for(uint8_t side = 0; side < 6; side++){
		//put all into one vector
		//to upload all at once
		packed_data.data.insert(
				packed_data.data.end(),
				out_data.at(side).begin(),
				out_data.at(side).end());
	}

	return packed_data;
}
