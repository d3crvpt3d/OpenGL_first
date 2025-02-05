#pragma once

typedef struct Camera{
	float x;
	float y;
	float z;
	float fovY;
	float yaw;
	float pitch;
	float far;
	float near;
	float aspectRatio;
} Camera;