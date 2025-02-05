#pragma once

typedef struct Camera{
	float x;
	float y;
	float z;
	float fovY;
	float yaw;
	float pitch;
	float front;
	float back;
	float aspectRatio;
} Camera;