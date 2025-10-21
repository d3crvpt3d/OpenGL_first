#version 410 core

in float vLight;

//texture
//in vec2 aTexCoord;
uniform sampler2D aTexture;

out vec4 FragColor;

void main() {
	
	//FragColor = vec4(vec3(gl_FragCoord.z), 1.0); //use for depth map
	
	//FragColor = vLight * texture2D(aTexture, aTexCoord);

	FragColor = vLight * vec4(0.5, 0.5, 0.5, 1.0);
}
