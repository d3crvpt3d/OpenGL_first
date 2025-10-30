#include "frustumCulling.h"
#include "chunkGeneration.h"
#include "main.h"
#include <cmath>

typedef struct{
	float x;
	float y;
	float z;
} vec3_t;

float dot(vec3_t a, vec3_t b){
	return 	a.x*b.x+
			a.y*b.y+
			a.z*b.z;
}

bool outOfFrustum(vec3i_t currChunk,
		int32_t lx,
		int32_t ly,
		int32_t lz,
		float viewX,
		float viewY,
		float viewZ){

	vec3_t view = {viewX, viewY, viewZ};


	vec3_t chunkVec = {(float) (lx - currChunk.x), (float) (ly - currChunk.y), (float) (lz - currChunk.z)};

	float cVecSize = sqrtf(	chunkVec.x*chunkVec.x+
							chunkVec.y*chunkVec.y+
							chunkVec.z*chunkVec.z);
	
	//TODO:change to good
	if(cVecSize < 2.0){
		return false;
	}


	vec3_t chunkNorm = {
		chunkVec.x / cVecSize,
		chunkVec.y / cVecSize,
		chunkVec.z / cVecSize
	};




	return dot(chunkNorm, view) < cosf(1.79*PI/4.0);
}
