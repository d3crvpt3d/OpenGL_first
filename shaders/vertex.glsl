#version 410 core

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_color;

uniform vec3 cameraPos;
uniform vec2 cameraDir; //TODO: include

out vec3 color;

void main() {
	color = vertex_color;
	gl_Position = vec4(vertex_position, 1.0);
}