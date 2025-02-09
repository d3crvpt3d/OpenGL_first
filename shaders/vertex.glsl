#version 410 core

#define LIGHTS 1;

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_color;
layout(location = 2) in vec3 vertex_normal;

//Camera values
uniform vec3 cameraPos;
uniform vec2 yaw_pitch;
uniform vec2 near_far;
uniform float f;
uniform float ratio;

//Lights
uniform vec3 LightPosition[LIGHTS];
uniform vec3 LightColor[LIGHTS];
uniform vec3 LightIntensity[LIGHTS];
uniform vec3 LightDirection[LIGHTS];


out vec3 color;
out vec3 normal;

void main() {

	/*-------- Phong Lighting --------*/

	cameraDirection = vec3(0.0, 0.0, 1.0);//TODO:
	reflection = LightDirection - 2 * (LightDirection * vertex_normal) * vertex_normal;

	for(int i = 0; i < LIGHTS; i++){
		color += LightIntensity[i] * ( LightColor * (normal * LightDirection) + pow((-cameraDirection * reflection), 25.0) );
	}

	/*-------- Camera projection --------*/

	cameraDirection = pitch * yaw * vec3(0.0, 0.0, 1.0);

	//translation
  vec3 tranVert = vertex_position - cameraPos;
  

	//pitch then yaw rotation
	mat3 pitch = mat3(
		1.0, 0.0, 								0.0,
		0.0, cos(-yaw_pitch.y), 	sin(-yaw_pitch.y),
		0.0, -sin(-yaw_pitch.y), 	cos(-yaw_pitch.y)
	);

	mat3 yaw = mat3(
		cos(yaw_pitch.x), 	0.0, 	sin(yaw_pitch.x),
		0.0, 								1.0, 	0.0,
		-sin(yaw_pitch.x), 	0.0, 	cos(yaw_pitch.x)
	);

	vec3 pitch_yawVert = pitch * yaw * tranVert;

	// projection matrix
	//calculate 2 times tan over two
	mat4 projection = mat4( //TODO: fix
		f,	0.0,	0.0,	0.0,
		0.0,	f,	0.0,	0.0,
		0.0,	0.0,	1.0,	1.0,
		0.0,	0.0,	0.0,	0.0
	);

	gl_Position = projection * vec4(pitch_yawVert, 1.0);
}