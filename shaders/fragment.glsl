#version 410 core

//texture
uniform sampler2D aTexture;

in vec2 vTexCoord;
in float vLight;

out vec4 FragColor;

void main() {
	
	//FragColor = vec4(vec3(gl_FragCoord.z), 1.0); //use for depth map
	
	FragColor = vLight * texture(aTexture, vTexCoord);

	//FragColor = vLight * vec4(0.5, 0.5, 0.5, 1.0);
}
