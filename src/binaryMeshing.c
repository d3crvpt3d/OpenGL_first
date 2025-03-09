#include "binaryMeshing.h"

void fillMemoryX(uint64_t *dest, uint16_t ***rawBlockData){

	for(uint8_t a = 0; a < CHUNK_WDH; a++){
		for(uint8_t b = 0; b < CHUNK_WDH; b++){
			for(uint8_t c = 0; c < CHUNK_WDH; c++){

				if(rawBlockData[a][b][c]){
					dest[a * CHUNK_WDH + b] |= 1 << c;
				}

			}
		}
	}
}

void fillMemoryY(uint64_t *dest, uint16_t ***rawBlockData){

	for(uint8_t a = 0; a < CHUNK_WDH; a++){
		for(uint8_t b = 0; b < CHUNK_WDH; b++){
			for(uint8_t c = 0; c < CHUNK_WDH; c++){

				if(rawBlockData[a][b][c]){
					dest[a * CHUNK_WDH + c] |= 1 << b;
				}

			}
		}
	}
}

void fillMemoryZ(uint64_t *dest, uint16_t ***rawBlockData){

	for(uint8_t a = 0; a < CHUNK_WDH; a++){
		for(uint8_t b = 0; b < CHUNK_WDH; b++){
			for(uint8_t c = 0; c < CHUNK_WDH; c++){

				if(rawBlockData[a][b][c]){
					dest[b * CHUNK_WDH + c] |= 1 << a;
				}

			}
		}
	}
}

//TODO: check if works
//binary mesh both positive and negative
void mesh(uint8_t *dest, uint64_t *source, uint8_t bitOffset){

	uint64_t tmp;

	//right face visible
	for(uint16_t a = 0; a < CHUNK_WDH * CHUNK_WDH; a++){
		tmp |= source[a] & ~(source[a] >> 1);
		
		for(uint8_t b = 0; b < sizeof(uint64_t); b++){
			
			if(tmp & (1 << b)){
				dest[a*CHUNK_WDH+b] |= 0x80 >> bitOffset;
			}		
		}
	}

	//left face visible
	for(uint16_t a = 0; a < CHUNK_WDH * CHUNK_WDH; a++){
		tmp |= source[a] & ~(source[a] << 1);
		
		for(uint8_t b = 0; b < sizeof(uint64_t); b++){
		
			if(tmp & (1 << b)){
				dest[a*CHUNK_WDH+b] |= 0x80 >> (bitOffset + 1);
			}		
		}
	}

}

//return block array where each bit is if face is visible from direction in {-x,+x,-y,+y,-z,+z}
uint8_t *createBinaryMesh(int32_t tmpX, int32_t tmpY, int32_t tmpZ, ChunkMap_t *chunkMap){
	
	uint64_t *memX = (uint64_t *) calloc(CHUNK_WDH * CHUNK_WDH, sizeof(uint64_t));
	uint64_t *memY = (uint64_t *) calloc(CHUNK_WDH * CHUNK_WDH, sizeof(uint64_t));
	uint64_t *memZ = (uint64_t *) calloc(CHUNK_WDH * CHUNK_WDH, sizeof(uint64_t));

	uint8_t *out = (uint8_t *) calloc(BLOCKS_PER_CHUNK, sizeof(uint8_t));

	fillMemoryX(memX, chunkMap_get(chunkMap, tmpX, tmpY, tmpZ)->blocks);
	fillMemoryY(memY, chunkMap_get(chunkMap, tmpX, tmpY, tmpZ)->blocks);
	fillMemoryZ(memZ, chunkMap_get(chunkMap, tmpX, tmpY, tmpZ)->blocks);

	mesh(out, memX, 0);
	mesh(out, memY, 2);
	mesh(out, memZ, 4);

	return out;
}