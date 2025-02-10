#version 410 core

in vec3 color;
in vec3 normal;
in vec3 cameraDirection;

out vec4 frag_color;

void main() {

	/*-------- Phong Shading --------*/

	/* only sun */
	
	frag_color = vec4(vec3(gl_FragCoord.z), 1.0);

}