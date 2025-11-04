#version 410 core

//binding 0
layout(location = 0) in vec3 aVertexPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTexCoord;

//binding 1
layout(location = 3) in vec3 aQuadPos; //pos
layout(location = 4) in uvec2 aQuadSize; //width & height
layout(location = 5) in uint aQuadType; //block type

//precomputed projecion Matrix
uniform mat4 projMatrix;
uniform vec3 camPos;

//output
out float vLight;
out vec2 vTexCoord;

void main() {


	//lod resize
	vec3 scaleVec = aQuadSize.x * aVertexPosition;

	vec3 scaledPos = aVertexPosition * scaleVec;


	//affine transformations
	vec3 worldPos = aQuadPos + scaledPos;

	vec4 cameraPos = vec4(worldPos - camPos, 1.0);

	gl_Position = projMatrix * cameraPos;


	//sunlight
	vLight = max(0.18, dot(aNormal, vec3(0.408248, 0.816497, 0.408248)));
	

	//offset texCoord.u by block type
	vTexCoord = vec2(aTexCoord.x + float(aQuadType & 0xFFFF) * 0.0625, aTexCoord.y);

}
