#pragma once
#include <glad/gl.h>

typedef struct Camera{
	GLfloat x;
	GLfloat y;
	GLfloat z;

	GLfloat f;
	
	GLfloat yaw;
	GLfloat pitch;
	
	GLfloat far;
	GLfloat near;
	
	GLfloat aspectRatio;
  GLfloat deltaTime;
} Camera;

typedef struct Light{
	GLfloat x;
	GLfloat y;
	GLfloat z;

	GLfloat direction[3];

	GLfloat rgb[3];

	GLfloat intensity;
} Light;