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
out vec3 vLight;
out vec2 vTexCoord;

void main() {


	//lod resize
	vec3 scaleVec = aQuadSize.x * aVertexPosition;

	vec3 scaledPos = aVertexPosition * scaleVec;


	//affine transformations
	vec3 worldPos = aQuadPos + scaledPos;

	vec4 cameraPos = vec4(worldPos - camPos, 1.0);

	gl_Position = projMatrix * cameraPos;


	//calculate light in vertex shader, because normals are the same on each face
	vec3 sunColor = vec3(1.0);
	vec3 sunDir = normalize(vec3(0.4336, 1.0, -0.5664));
	vec3 ambient = vec3(0.125);

	//for now no specular lighting
	vLight = ambient + clamp(sunColor * dot(sunDir, aNormal), 0.0, 1.0);
	

	//offset texCoord.u by block type
	vTexCoord = vec2(aTexCoord.x + float(aQuadType & 0xFFFF) * 0.0625, aTexCoord.y);

}
