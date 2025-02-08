#version 410 core

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_color;

uniform vec3 cameraPos;
uniform vec2 yaw_pitch;
uniform vec2 near_far;
uniform float fovY;
uniform float ratio;

out vec3 color;

void main() {
	color = vertex_color;

	//translation
  vec3 tranVert = vertex_position - cameraPos;
  

	//pitch then yaw rotation
	mat3 pitch = mat3(
		1.0, 0.0, 0.0,
		0.0, cos(-yaw_pitch.y), sin(-yaw_pitch.y),
		0.0, -sin(-yaw_pitch.y), cos(-yaw_pitch.y)
	);

	vec3 pitchVert = pitch * tranVert;

	mat3 yaw = mat3(
		cos(yaw_pitch.x), 0.0, sin(yaw_pitch.x),
		0.0, 1.0, 0.0,
		-sin(yaw_pitch.x), 0.0, cos(yaw_pitch.x)
	);

	vec3 pitch_yawVert = yaw * pitchVert;

	// projection matrix
	//calculate 2 times tan over two
	mat4 projection = mat4( //TODO: fix
		1.0,	0.0,	0.0,	0.0,
		0.0,	1.0,	0.0,	0.0,
		0.0,	0.0,	1.0,	1.0,
		0.0,	0.0,	0.0,	0.0
	);

	gl_Position = projection * vec4(pitch_yawVert, 1.0);
}