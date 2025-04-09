#include "voxelTrace.h"
#include "chunkMap.h"

#include <stdio.h>
#include <math.h>
#include <float.h>

typedef struct{
    float x;
    float y;
    float z;
} vec3;

//squared distance
float dist_sq(vec3 a, vec3 b){
    return powf(a.x - b.x, 2.f) + powf(a.y - b.y, 2.f) + powf(a.z - b.z, 2.f);
}


uint16_t getBlock(vec3 *pos, ChunkMap_t *chunkMap){
    
    int32_t x = (int32_t) pos->x;
    int32_t y = (int32_t) pos->y;
    int32_t z = (int32_t) pos->z;

    return chunkMap_getBlock(chunkMap, x, y, z);
}

//based heavily on "A Fast Voxel Traversal Algorithm for Ray Tracing" by Amanatides and Woo
//TODO: fix
void update_lookingAt(float xyz[3], float yaw_pitch[2], vec3i_t *break_block, vec3i_t *place_block, float maxDistance, ChunkMap_t *chunkMap){
    
    //init defaults
    vec3 pos = {xyz[0], xyz[1], xyz[2]};
    
    //convert yaw/pitch to xyz
    vec3 dir = {cosf(yaw_pitch[1]) * sinf(yaw_pitch[0]) + FLT_EPSILON, -sinf(yaw_pitch[1]) + FLT_EPSILON, cosf(yaw_pitch[0]) * cosf(yaw_pitch[1]) + FLT_EPSILON};
    //fprintf(stderr,"dir: %f, %f, %f\n", dir.x, dir.y, dir.z);
    
    float nX = floorf(pos.x) + 0.5f * dir.x/fabsf(dir.x) + 0.5f;
    float nY = floorf(pos.y) + 0.5f * dir.y/fabsf(dir.y) + 0.5f;
    float nZ = floorf(pos.z) + 0.5f * dir.z/fabsf(dir.z) + 0.5f;
    
    //gradient with bases on other
    float mx_y = dir.x/dir.y;
    float mx_z = dir.x/dir.z;
    float my_x = dir.y/dir.x;
    float my_z = dir.y/dir.z;
    float mz_x = dir.z/dir.x;
    float mz_y = dir.z/dir.y;
    
    // calculate deltas etc
    float stepX = dir.x/fabsf(dir.x);
    float stepY = dir.y/fabsf(dir.y);
    float stepZ = dir.z/fabsf(dir.z);
    
    vec3 deltaX = {stepX, stepX * my_x, stepX * mz_x};
    vec3 deltaY = {stepY * mx_y, stepY, stepX * mz_y};
    vec3 deltaZ = {stepZ * mx_z, stepZ * my_z, stepZ};
    
    vec3 newPos = {pos.x, pos.y, pos.z};
    
    // get intersection pos for each nearest new block
    vec3 nextX = {nX, (nX - pos.x) * my_x + pos.y, (nX - pos.x) * mz_x + pos.z};
    vec3 nextY = {(nY - pos.y) * mx_y + pos.x, nY, (nY - pos.y) * mz_y + pos.z};
    vec3 nextZ = {(nZ - pos.z) * mx_z + pos.x, (nZ - pos.z) * my_z + pos.y, nZ};
    
    // go to nearest block intersection
    if(dist_sq(pos, nextX) < dist_sq(pos, nextY)){
        if(dist_sq(pos, nextX) < dist_sq(pos, nextZ)){
            newPos.x = nextX.x;
            newPos.y = nextX.y;
            newPos.z = nextX.z;
        }else{
            newPos.x = nextZ.x;
            newPos.y = nextZ.y;
            newPos.z = nextZ.z;
        }
    }else{
        if(dist_sq(pos, nextY) < dist_sq(pos, nextZ)){
            newPos.x = nextY.x;
            newPos.y = nextY.y;
            newPos.z = nextY.z;
        }else{
            newPos.x = nextZ.x;
            newPos.y = nextZ.y;
            newPos.z = nextZ.z;
        }
    }
    
    vec3 last = {newPos.x, newPos.y, newPos.z};
    vec3 ndlast = {newPos.x, newPos.y, newPos.z};
    
    //update loop
    while(dist_sq(pos, newPos) < powf(maxDistance, 2.f)){
        
        if(getBlock(&newPos, chunkMap)){
            place_block->x = floorf(ndlast.x);
            place_block->y = floorf(ndlast.y);
            place_block->z = floorf(ndlast.z);
            
            break_block->x = floorf(last.x);
            break_block->y = floorf(last.y);
            break_block->z = floorf(last.z);

            //fprintf(stderr,"looking at:%d,%d,%d\n", break_block->x, break_block->y, break_block->z);
            return;
        }
        
        ndlast.x = last.x;
        ndlast.y = last.y;
        ndlast.z = last.z;
        last.x = newPos.x;
        last.y = newPos.y;
        last.z = newPos.z;
        
        if(dist_sq(newPos, nextX) < dist_sq(newPos, nextY)){
            if(dist_sq(newPos, nextX) < dist_sq(newPos, nextZ)){
                newPos.x = nextX.x;
                newPos.y = nextX.y;
                newPos.z = nextX.z;
                
                nextX.x += deltaX.x;
                nextX.y += deltaX.y;
                nextX.z += deltaX.z;
            }else{
                newPos.x = nextZ.x;
                newPos.y = nextZ.y;
                newPos.z = nextZ.z;
                
                nextZ.x += deltaZ.x;
                nextZ.y += deltaZ.y;
                nextZ.z += deltaZ.z;
            }
        }else{
            if(dist_sq(newPos, nextY) < dist_sq(newPos, nextZ)){
                newPos.x = nextY.x;
                newPos.y = nextY.y;
                newPos.z = nextY.z;
                
                nextY.x += deltaY.x;
                nextY.y += deltaY.y;
                nextY.z += deltaY.z;
            }else{
                newPos.x = nextZ.x;
                newPos.y = nextZ.y;
                newPos.z = nextZ.z;
                
                nextZ.x += deltaZ.x;
                nextZ.y += deltaZ.y;
                nextZ.z += deltaZ.z;
            }
        }
    }

    //looking at nothing
    place_block->x = floorf(pos.x);
    place_block->y = floorf(pos.y);
    place_block->z = floorf(pos.z);
    
    break_block->x = floorf(pos.x);
    break_block->y = floorf(pos.y);
    break_block->z = floorf(pos.z);
}