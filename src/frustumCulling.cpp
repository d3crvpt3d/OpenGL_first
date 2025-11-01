#include "frustumCulling.h"
#include "chunkGeneration.h"
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
		float fov_view_dot,
		int32_t lx,
		int32_t ly,
		int32_t lz,
		float viewX,
		float viewY,
		float viewZ){

	vec3_t view = {viewX, viewY, viewZ};

	//add 1 to chunkVec in view direction
	//to offset vector
	vec3_t chunkVec = {
		(float) (lx - currChunk.x) + viewX,
		(float) (ly - currChunk.y) + viewY,
		(float) (lz - currChunk.z) + viewZ};

	float cVecSize = sqrtf(	chunkVec.x*chunkVec.x+
							chunkVec.y*chunkVec.y+
							chunkVec.z*chunkVec.z);

	vec3_t chunkNorm = {
		chunkVec.x / cVecSize,
		chunkVec.y / cVecSize,
		chunkVec.z / cVecSize
	};



	return dot(chunkNorm, view) < fov_view_dot;
}
