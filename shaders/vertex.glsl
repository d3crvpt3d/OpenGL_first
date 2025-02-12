#version 410 core

#define LIGHTS 1

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_color;

//Model View Projection Matrix
uniform mat4 VPmatrix;

//Lights
uniform vec3 LightPosition[LIGHTS];
uniform vec3 LightColor[LIGHTS];
uniform float LightIntensity[LIGHTS];
uniform vec3 LightDirection[LIGHTS];
uniform uint numLights;


out vec3 color;

void main() {

	color = vertex_color;
	/*-------- Phong Lighting --------*/

	/*-------- Camera projection --------*/

	gl_Position = vec4((VPmatrix * vec4(vertex_position, 1.0)).xyz, 1.0);
}