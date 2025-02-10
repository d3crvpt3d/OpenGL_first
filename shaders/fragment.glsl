#version 410 core

in vec3 color;
in vec3 normal;
in vec3 cameraDirection;

out vec4 FragColor;

void main() {

	/*-------- Phong Shading --------*/

	/* only sun */
	
	FragColor = vec4(color, 1.0);

}