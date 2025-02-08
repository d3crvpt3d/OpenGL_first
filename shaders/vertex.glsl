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
  
	// rotation
	mat3 rotation = mat3(
		cos(yaw_pitch.x), sin(yaw_pitch.y) * sin(yaw_pitch.x), -(cos(yaw_pitch.y) * sin(yaw_pitch.x)),
		0.0, cos(yaw_pitch.y), sin(yaw_pitch.y),
		sin(yaw_pitch.x), -(cos(yaw_pitch.x) * sin(yaw_pitch.y)), cos(yaw_pitch.y) * cos(yaw_pitch.x)
	);

	vec3 rotVert = rotation * tranVert;

	// projection matrix
	//calculate 2 times tan over two
	float tot = tan(fovY/2);

	mat4 projection = mat4( //TODO: fix
		1.0 / (ratio * tot),	0.0,		0.0, 																0.0,
		0.0, 							1.0 / tot, 	0.0, 																0.0,
		0.0, 							0.0,		near_far.y / (near_far.y - near_far.x),	-(near_far.y * near_far.x) / (near_far.y - near_far.x),
		0.0, 							0.0, 		1.0,																		0.0
	);

	gl_Position = projection * vec4(tranVert, 1.0);
}