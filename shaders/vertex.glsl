#version 410 core

#define LIGHTS 1

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_color;

//Model View Projection Matrix
uniform vec2 cam_dir;
uniform vec3 cam_pos;

//non freq coord
uniform float f;
uniform float ratio;
uniform float near;
uniform float far;

//chunk generation
uniform vec3 vertex_normal; //normal vector for each side
uniform vec3 chunkOffset;	//offset of chunks (0, 0, 0)
uniform vec3 offsets[512];//postitional chunk offsets
uniform bool visible[512];//discord if false for given index

out vec3 color;

void main() {

	color = dot(vertex_normal, vec3(-1.0, -2.0, -1.0));

	vec3 trans = vertex_position - cam_pos;

	mat4 view1 = mat4(
		cos(cam_dir.x), 0.0, sin(cam_dir.x), 0.0,
		0.0, 1.0, 0.0, 0.0,
		-sin(cam_dir.x), 0.0, cos(cam_dir.x), 0.0,
		0.0, 0.0, 0.0, 1.0
	);

	mat4 view2 = mat4(
		1.0, 0.0, 					 0.0, 					  0.0,
		0.0, cos(cam_dir.y), -sin(cam_dir.y),  0.0,
		0.0, sin(cam_dir.y), cos(cam_dir.y), 0.0,
		0.0, 0.0, 					 0.0, 					  1.0
	);

	mat4 proj = mat4(
		f/ratio, 0.0, 0.0, 0.0,
		0.0, f, 0.0, 0.0,
		0.0, 0.0, -far/(far-near), 1.0,
		0.0, 0.0, near*far/(far-near), 0.0
	);

	gl_Position = proj * view2 * view1 * vec4(trans, 1.0);
}