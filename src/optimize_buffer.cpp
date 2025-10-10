#include "include/optimize_buffer.h"
#include "include/chunkMap.h"
#include <cstdint>
#include <vector>

#define SKIP_OPTIMIZED 1

std::vector<Block_t> gen_optimized_buffer(Chunk_t &chunk){


	std::vector<Block_t> out_data;

	//DEBUG
	if(SKIP_OPTIMIZED){

		for(uint8_t z = 0; z < 64; z++){
			for(uint8_t y = 0; y < 64; y++){
				for(uint8_t x = 0; x < 64; x++){

					// Die alte, unsichere Bitfield-Zuweisung:
					// Block_t tmp = {x, y, z, 0, (GLuint) chunk.blocks[z][y][x]};
					// out_data.push_back(tmp);

					// DIE NEUE, SICHERE METHODE:
					uint32_t type = (uint32_t)chunk.blocks[z][y][x];

					// Baue die 32-bit-Zahl manuell mit Bit-Operationen zusammen
					// Exakt spiegelverkehrt zur Logik im Shader!
					uint32_t packed_data = 0;
					packed_data |= ( (uint32_t)x & 63 ) << 26; // x in die höchsten 6 Bits
					packed_data |= ( (uint32_t)y & 63 ) << 20; // y in die nächsten 6 Bits
					packed_data |= ( (uint32_t)z & 63 ) << 14; // z in die nächsten 6 Bits
															   // break_progress (4 bits) überspringen wir, bleibt 0
					packed_data |= ( type & 1023 );          // type in die niedrigsten 10 Bits

					// Konvertiere die fertige Zahl in den Block_t-Typ und füge sie hinzu
					out_data.push_back(*reinterpret_cast<Block_t*>(&packed_data));
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

					Block_t tmp = {x,
						y,
						z,
						0,
						(GLuint) chunk.blocks[z][y][x]};

					out_data.push_back(tmp);
				
				}

			}
		}
	}

	chunk.bufferSize = out_data.size(); //give information about buffer size

	return out_data;
}
