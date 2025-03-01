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
#define CHUNKDATA E:\Code\Projects\OpenGL\opengl_glfw_1\include\main.h

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