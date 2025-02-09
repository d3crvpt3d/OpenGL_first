#version 410 core

in vec3 color;
in vec3 normal;
in vec3 cameraDirection;

out vec4 frag_color;

void main() {

	/*-------- Phong Shading --------*/

	/* only sun */
	
	frag_color = color;

}