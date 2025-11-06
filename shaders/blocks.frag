#version 410 core

//texture
uniform sampler2D aTexture;
uniform samplerCube aSkybox;

in vec2 vTexCoord;

in vec3 vNormal;
in vec3 vViewDir;

out vec4 FragColor;

void main() {
	
	//tex color
	vec4 blockColor = texture(aTexture, vTexCoord);

	//sample cubemap
	vec4 skyLight = texture(aSkybox, vNormal);

	FragColor = blockColor * skyLight;

}
