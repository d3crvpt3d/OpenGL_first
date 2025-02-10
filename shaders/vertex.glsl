#version 410 core

#define LIGHTS 1

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_color;
layout(location = 2) in vec3 vertex_normal;

//Camera values
uniform vec3 cameraPos;
uniform vec2 yaw_pitch;
uniform vec2 near_far;
uniform float f;
uniform float ratio;

//Lights
uniform vec3 LightPosition[LIGHTS];
uniform vec3 LightColor[LIGHTS];
uniform float LightIntensity[LIGHTS];
uniform vec3 LightDirection[LIGHTS];
uniform uint numLights;


out vec3 color;
out vec3 normal;

void main() {

	color = vertex_color;
	/*-------- Phong Lighting --------*/

	/*-------- Camera projection --------*/

	//translation
  vec3 tranVert = vertex_position - cameraPos;
  

	//pitch then yaw rotation
	mat3 pitch = mat3(
		1.0, 0.0, 								0.0,
		0.0, cos(-yaw_pitch.y), 	sin(-yaw_pitch.y),
		0.0, -sin(-yaw_pitch.y), 	cos(-yaw_pitch.y)
	);

	mat3 yaw = mat3(
		cos(yaw_pitch.x), 	0.0, 	sin(yaw_pitch.x),
		0.0, 								1.0, 	0.0,
		-sin(yaw_pitch.x), 	0.0, 	cos(yaw_pitch.x)
	);

	vec3 pitch_yawVert = pitch * yaw * tranVert;

	// projection matrix
	mat4 projection = mat4( //TODO: fix
		vec4(f, 	0.0,	0.0, 0.0),
		vec4(0.0, f,		0.0, 0.0),
		vec4(0.0, 0.0, 	near_far.y / (near_far.y - near_far.x), 1.0),
		vec4(0.0, 0.0, 	near_far.x * near_far.y / (near_far.y - near_far.x), 0.0)
	);

	gl_Position = projection * vec4(pitch_yawVert, 1.0);
}