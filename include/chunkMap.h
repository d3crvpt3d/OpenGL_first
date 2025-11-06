#pragma once

#include <array>
#include <cstdint>
#include <mutex>
#include <stdlib.h>
#include <stdint.h>

/* GLOBALS */

extern std::mutex chunkMap_mutex;

/* DEFINES */
#define RENDERDISTANCE 4 //8*64 = 32*16
#define CHUNK_WDH 64	//chunk width/depth/height
#define BLOCKS_PER_CHUNK (CHUNK_WDH * CHUNK_WDH * CHUNK_WDH)

#define RENDERSPAN (2*RENDERDISTANCE+1) //from x-renderdistance to x+renderdistance in chunks
#define CACHE_SPAN 32
#define CHUNKS RENDERSPAN * RENDERSPAN * RENDERSPAN

/* STRUCTS */

//a Chunk in CPU memory is a 3D uint32_t array where each int is the block type
typedef struct Chunk{
	std::array<std::array<std::array<uint16_t, 64>, 64>, 64> blocks = {0};
	int32_t x;
	int32_t y;
	int32_t z;
	uint32_t bufferSize;
	uint8_t modified: 1;
	uint8_t initialized: 1;
} Chunk_t;


/* FUNCTIONS */

//real modulus
uint32_t mod(int32_t a, int32_t b);

class ChunkMap{
	std::array<std::array<std::array<Chunk_t, RENDERSPAN>, RENDERSPAN>, RENDERSPAN> chunks;

	public:

		ChunkMap(){
			chunks = {0};
		}
		
		//wird oft aufgerufen
		inline uint16_t getBlockAtWorldPos(int32_t worldX, int32_t worldY, int32_t worldZ){
			int32_t chunkX = worldX >> 6;
			int32_t chunkY = worldY >> 6;
			int32_t chunkZ = worldZ >> 6;

			Chunk_t *chunk = at(chunkX, chunkY, chunkZ);

			//check if right chunk
			if(!chunk->initialized ||
					chunk->x != chunkX ||
					chunk->y != chunkY ||
					chunk->z != chunkZ){
				return 0;
			}

			//optimized for 2^n modulus
			return chunk->blocks[worldZ & 63][worldY & 63][worldX & 63];
		}

		//modulus index
		Chunk_t *at(int32_t x, int32_t y, int32_t z){
			return &chunks
				.at(mod(z, RENDERSPAN))
				.at(mod(y, RENDERSPAN))
				.at(mod(x, RENDERSPAN));
		}
};
