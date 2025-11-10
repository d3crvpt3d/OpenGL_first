#version 410 core

//texture
uniform sampler2D aTexture;
uniform samplerCube aSkybox;

in vec2 vTexCoord;
in vec3 vNormal;
in vec3 vViewDir;

in float vDistanceToFrag;

out vec4 FragColor;

void main() {
	
	//tex color
	vec4 blockColor = texture(aTexture, vTexCoord);

	//sample cubemap
	vec4 skyLight = texture(aSkybox, vNormal);

	//float fogBase = 1.00135471989211;//2^(1/512)
	float fogBase = 1.00271127505;//2^(2/512)
	
	float fogFactor = pow(fogBase, vDistanceToFrag) - 3.0;

	//vec4 fogColor = vec4(0.239, 0.267, 0.337, 1.0);
	vec4 fogColor = texture(aSkybox, vViewDir);

	FragColor = mix(blockColor * skyLight, fogColor, clamp(fogFactor, 0.0, 1.0));

}
