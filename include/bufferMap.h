#pragma once

#include "chunkMap.h"
#include "main.h"
#include <cstdint>
#include <sys/types.h>

//map of GLuint'S for VRAM buffers
class bufferMap{

	std::array<std::array<GLuint,CHUNKS>,2> arrays;

	public:
		std::array<std::array<GLuint,CHUNKS>,2> &raw(){
			return arrays;
		}	

		GLuint &atVAO(int32_t x, int32_t y, int32_t z){
			
			uint32_t x2 = mod(x, RENDERSPAN);
			uint32_t y2 = mod(y, RENDERSPAN);
			uint32_t z2 = mod(z, RENDERSPAN);

			//curren VAO
			return arrays[0][RENDERSPAN*RENDERSPAN*z2+RENDERSPAN*y2+x2];
		}

		GLuint &atVBO(int32_t x, int32_t y, int32_t z){
			
			uint32_t x2 = mod(x, RENDERSPAN);
			uint32_t y2 = mod(y, RENDERSPAN);
			uint32_t z2 = mod(z, RENDERSPAN);

			//curren VAO
			return arrays[1][RENDERSPAN*RENDERSPAN*z2+RENDERSPAN*y2+x2];
		}

};
