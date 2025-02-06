#version 410 core

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_color;

uniform vec3 cameraPos;
uniform vec2 cameraDir; //TODO: include

out vec3 color;

void main() {
	color = vertex_color;

  vec3 newVert = vec3(vertex_position - cameraPos);
  
	gl_Position = vec4(
    newVert.xy/newVert.z,
    1.0,
    1.0
    );
}