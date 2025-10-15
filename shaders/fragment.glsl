#version 410 core

in vec3 color;
in vec3 normal;
in vec3 cameraDirection;

//texture
in float aLight;
in vec2 aTexCoord;
uniform sampler2D aTexture;

out vec4 FragColor;

void main() {
	
	//FragColor = vec4(vec3(gl_FragCoord.z), 1.0); //use for depth map
	//FragColor = vec4(color, 1.0);
	FragColor = aLight * texture2D(aTexture, aTexCoord);
}
