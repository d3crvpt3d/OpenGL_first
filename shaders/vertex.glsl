#version 410 core

layout(location = 0) in vec3 aVertexPosition;

layout(location = 3) in vec3 aQuadPos;
layout(location = 4) in uvec2 aQuadSize; //width & height
layout(location = 5) in uint aQuadType; //block type

//face currently drawing
uniform uint face; //-x,+x,-y,+y,-z,+z

//Model View Projection Matrix
uniform vec2 cam_dir;
uniform vec3 cam_pos;

//non freq coord
uniform float f;
uniform float ratio;
uniform float near;
uniform float far;

out float aLight;
out vec2 aTexCoord;

void main() {

	//sunlight
	aLight = max(0.2, dot(aVertexNormal, vec3(0.408248, 0.816497, 0.408248)));
	
	//offset texCoord.u by block type
	aTexCoord = vec2(inTexCoord.x + float(aBlockData & 1023) * 0.0625, inTexCoord.y);

	vec3 newPos = aVertexPosition + aBlockPos;

	vec3 trans = newPos - cam_pos;

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

	gl_Position = proj * view2 * view1 * vec4(trans, 1.0);

	if( (aBlockData & 1023) == 0 ){
		gl_Position = vec4(0.0, 0.0, -10.0, 1.0);
	}
}
