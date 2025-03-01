#pragma once

#include <pthread.h>
#include <stdint.h>
#include "glad/gl.h"
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "chunkGeneration.h"


#define WIDTH 	1920
#define HEIGHT 	1080
#define FLYSPEED 2.0
#define RADPERPIXEL 0.00418879f //mouse sensitivity
#define PI 3.1415927f
#define LIGHTS 1 //num lights

typedef struct Camera{
	GLfloat xyz[3];

	GLfloat f;
	
	GLfloat yaw_pitch[2];
	
	GLfloat near_far[2];
	
	GLfloat aspectRatio;
  GLfloat deltaTime;
} Camera;

typedef struct Light{
	GLfloat xyz[3];

	GLfloat direction[3];

	GLfloat rgb[3];

	GLfloat intensity;
} Light;

void generateSpawnLocation();
void addNewChunkJob(int32_t lastX, int32_t lastY, int32_t lastZ, int32_t currX, int32_t currY, int32_t currZ);