#include "chunkMap.h"
#include <cstdint>

//b has to be in 2^n
uint32_t modulo_n2(int32_t a, int32_t b){
	return a & (b-1);
}

uint32_t mod(int32_t a, int32_t b){
	return ((a % b) + b) % b;
}
