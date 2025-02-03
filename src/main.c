#include "glad/gl.h"
#include <GLFW/glfw3.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define WIDTH 	1200
#define HEIGHT 	675

int main(){

	GLFWwindow *window;
	
	if(!glfwInit()){
		return -1;
	}

	window = glfwCreateWindow(WIDTH, HEIGHT, "Test Title!", NULL, NULL);

	if(!window){
		glfwTerminate();
		return -1;
	}

	//cap to monitor fps
	glfwSwapInterval(1);

	glfwMakeContextCurrent(window);

	int version_glad = gladLoadGL(glfwGetProcAddress);
	if ( version_glad == 0 ) {
    fprintf( stderr, "ERROR: Failed to initialize OpenGL context.\n" );
    return 1;
  }
	printf( "Loaded OpenGL %i.%i\n", GLAD_VERSION_MAJOR( version_glad ), GLAD_VERSION_MINOR( version_glad ) );
	printf( "Renderer: %s.\n", glGetString( GL_RENDERER ) );
  printf( "OpenGL version supported %s.\n", glGetString( GL_VERSION ) );
	
	
	/* create VBO */
	float points[] = {
		0.0f,  0.5f,  0.0f,
   	0.5f, -0.5f,  0.0f,
   -0.5f, -0.5f,  0.0f
	};

	float colors[] = {
  1.0f, 0.0f,  0.0f,
  0.0f, 1.0f,  0.0f,
  0.0f, 0.0f,  1.0f
};

	GLuint points_vbo = 0;
	glGenBuffers( 1, &points_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, points_vbo );
	glBufferData( GL_ARRAY_BUFFER, 9 * sizeof( float ), points, GL_STATIC_DRAW );
	
	GLuint colors_vbo = 0;
	glGenBuffers( 1, &colors_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, colors_vbo );
	glBufferData( GL_ARRAY_BUFFER, 9 * sizeof( float ), colors, GL_STATIC_DRAW );


	/* create VAO */
	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	/* enable */
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	/* Shaders */

	/* Load Shaders */

	const char* vertex_shader =
	"#version 410 core\n"
	"layout(location = 0) in vec3 vertex_position;"
	"layout(location = 1) in vec3 vertex_color;"
	"out vec3 color;"
	"void main() {"
	"	color = vertex_color;"
	"	gl_Position = vec4( vertex_position, 1.0 );"
	"}";


	const char* fragment_shader =
	"#version 410 core\n"
	"in vec3 color;"
	"out vec4 frag_color;"
	"void main() {"
	"  frag_color = vec4( color, 1.0 );"
	"}";


	/* Link Shaders */
	GLuint vs = glCreateShader( GL_VERTEX_SHADER );
	glShaderSource( vs, 1, &vertex_shader, NULL );
	glCompileShader( vs );

	GLuint fs = glCreateShader( GL_FRAGMENT_SHADER );
	glShaderSource( fs, 1, &fragment_shader, NULL );
	glCompileShader( fs );

	GLuint shader_program = glCreateProgram();
	glAttachShader( shader_program, vs );
	glAttachShader( shader_program, fs );
	
	glLinkProgram( shader_program );

	/* MAIN LOOP */
	while ( !glfwWindowShouldClose( window ) ) {
  	// Update window events.
  	glfwPollEvents();
  
  	// Wipe the drawing surface clear.
  	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  	// Put the shader program, and the VAO, in focus in OpenGL's state machine.
		float timeValue = glfwGetTime();
  	glUseProgram( shader_program );
  	glBindVertexArray( vao );

  	// Draw points 0-3 from the currently bound VAO with current in-use shader.
  	glDrawArrays( GL_TRIANGLES, 0, 3 );
  
  	// Put the stuff we've been drawing onto the visible area.
  	glfwSwapBuffers( window );
	}

	glfwTerminate();

	return 0;
}