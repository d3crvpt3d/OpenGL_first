#include "optimize_buffer.h"
#include "chunkGeneration.h"
#include "chunkMap.h"
#include <array>
#include <cstdint>
#include <vector>

#define TRANSPARENT_MASK 0x80 //uint16_t

void *face_offset[2][6];

std::array<std::vector<QuadGPU_t>, 6> gen_optimized_buffer(ChunkMap &map, int32_t x, int32_t y, int32_t z){

	std::vector<QuadGPU_t> out_data;

	bool visited [6][64][64][64] = {false}; //face,z,y,x

	for(uint8_t z = 0; z < 64; z++){
		for(uint8_t y = 0; y < 64; y++){
			for(uint8_t x = 0; x < 64; x++){
	
				


			}
		}
	}

}
