#pragma once
#include <glad/gl.h>

typedef struct Camera{
	GLfloat x;
	GLfloat y;
	GLfloat z;
	GLfloat fovY;
	GLfloat yaw;
	GLfloat pitch;
	GLfloat far;
	GLfloat near;
	GLfloat aspectRatio;
  GLfloat deltaTime;
} Camera;