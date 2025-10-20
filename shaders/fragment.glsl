#version 410 core

in float aLight;

//texture
in vec2 aTexCoord;
uniform sampler2D aTexture;

out vec4 FragColor;

void main() {
	
	//FragColor = vec4(vec3(gl_FragCoord.z), 1.0); //use for depth map
	
	//FragColor = aLight * texture2D(aTexture, aTexCoord);

	FragColor = vec4(1.0, 0.0, 1.0, 1.0);
}
