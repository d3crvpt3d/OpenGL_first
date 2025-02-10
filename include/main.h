#pragma once
#include <glad/gl.h>

#define WIDTH 	1920
#define HEIGHT 	1080
#define FLYSPEED 2.0
#define RADPERPIXEL 0.00418879f //mouse sensitivity
#define PI 3.1415927f
#define LIGHTS 1

typedef struct Camera{
	GLfloat xyz[3];

	GLfloat f;
	
	GLfloat yaw;
	GLfloat pitch;
	
	GLfloat far;
	GLfloat near;
	
	GLfloat aspectRatio;
  GLfloat deltaTime;
} Camera;

typedef struct Light{
	GLfloat xyz[3];

	GLfloat direction[3];

	GLfloat rgb[3];

	GLfloat intensity;
} Light;