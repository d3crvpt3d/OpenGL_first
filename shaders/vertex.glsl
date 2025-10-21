#version 410 core

//binding 0
layout(location = 0) in vec3 aVertexPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTexCoord;

//binding 1
layout(location = 3) in vec3 aQuadPos; //pos
layout(location = 4) in uvec2 aQuadSize; //width & height
layout(location = 5) in uint aQuadType; //block type

//Model View Projection Matrix
uniform vec3 cam_pos;
uniform vec2 cam_dir;
uniform float f;
uniform float ratio;
uniform float near;
uniform float far;

//output
out float vLight;
//out vec2 vTexCoord;
//out float vTexLayer;

void main() {

	vec3 worldPos = aVertexPosition + aQuadPos;

	vec4 trans = vec4(worldPos - cam_pos, 1.0);

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
		f, 0.0, 0.0, 0.0,
		0.0, f*ratio, 0.0, 0.0,
		0.0, 0.0, (far+near)/(far-near), 1.0,
		0.0, 0.0, -2.0*far*near/(far-near), 0.0
	);

	gl_Position = proj * view2 * view1 * trans;

	//sunlight
	vLight = max(0.2, dot(aNormal, vec3(0.408248, 0.816497, 0.408248)));
	
	//TODO
	//offset texCoord.u by block type
	//vTexCoord = vec2(inTexCoord.x + float(aQuadType & 0xFFFF) * 0.0625, inTexCoord.y);
	//vTexLayer = float(aQuadType);

}
