#version 410 core

layout(location = 0) in vec3 vertex_position;
layout(location = 3) in uint aBlockType;//TODO: change

//Model View Projection Matrix
uniform vec2 cam_dir;
uniform vec3 cam_pos;

//non freq coord
uniform float f;
uniform float ratio;
uniform float near;
uniform float far;

//chunk generation
uniform vec3 face_normal; //normal vector for each side
uniform vec3 chunk_pos;

out vec3 color;

void main() {

	if(aBlockType == 0){
		gl_Position = vec4(0.0, 0.0, -10.0, 1.0);
		return;
	}

	color = vec3(0.5, 0.5, 0.5);

	vec3 block_pos = vec3(
		(gl_InstanceID >> 0 ) & 0x3F,
		(gl_InstanceID >> 6 ) & 0xF,
		(gl_InstanceID >> 10) & 0x3F
		);

	vec3 newPos = vertex_position + chunk_pos + block_pos;



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