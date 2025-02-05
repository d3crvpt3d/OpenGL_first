#include "glad/gl.h"
#include <GLFW/glfw3.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define WIDTH 	1200
#define HEIGHT 	675

void key_callback(GLFWwindow *, int , int, int, int);

void updateProjectionMatrix(GLdouble *matrix, double deltaTime);

int main(){

	GLFWwindow *window;
	
	if(!glfwInit()){
		return -1;
	}

	/* GLFW Window Hints */
	glfwWindowHint(GLFW_SAMPLES, 4);
	//glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER , GLFW_TRUE);

	/* Create GLFW window */
	window = glfwCreateWindow(WIDTH, HEIGHT, "Test Title!", NULL, NULL);

	if(!window){
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	//1 = cap vsync to monitor fps
	glfwSwapInterval(1);

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
  	1.0f,  0.0f,  0.0f,
  	0.0f,  1.0f,  0.0f,
  	0.0f,  0.0f,  1.0f
	};

	float camera[] = {
		0.0f, -1.0f,  0.0f, // x,y,z
		0.0f, 0.0f					//yaw, pitch
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


	/* Load Shaders */

	const char* vertex_shader =
	"#version 410 core\n"
	"layout(location = 0) in vec3 vertex_position;"
	"layout(location = 1) in vec3 vertex_color;"
	"uniform mat4 projection_matrix;"
	"out vec3 color;"
	"void main() {"
	"	color = vertex_color;"
	"	gl_Position = vec4(vertex_position, 1.0);" //TODO: fix projection_matrix
	"}";


	const char* fragment_shader =
	"#version 410 core\n"
	"in vec3 color;"
	"out vec4 frag_color;"
	"void main() {"
	"	frag_color = vec4(color, 1.0);"
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


	//set up Input callbacks
	glfwSetKeyCallback(window, key_callback);

	double currTime = glfwGetTime();
	double lastTime;
	double deltaTime;

	/* get location of matrix in shader */
	GLint matrix_location = glGetUniformLocation(shader_program, "projection_matrix");

	/* Init as Idention Matrix */
	GLdouble projMatrix[] = {
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0
		};

	double title_cd = 0.2; //Update title only every 200ms (if changed change reset value in main loop)
	/* MAIN LOOP */
	while ( !glfwWindowShouldClose( window ) ) {
		
		/* Calculate delta Time of last frame */
		lastTime = currTime;
		currTime = glfwGetTime();
		deltaTime = currTime - lastTime;
		
		// set fps as title
		title_cd -= deltaTime;
		if(title_cd <= 0.0 && deltaTime > 0.0 ){
			double fps = 1.0 / deltaTime;
			char tmp[16];
			snprintf(tmp, sizeof(tmp), "FPS: %.2lf", fps);
			glfwSetWindowTitle(window, tmp);
			title_cd = 0.2; //reset value of title cd
		}
  	
		// Update window events.
  	glfwPollEvents();

  	// Wipe the drawing surface clear.
  	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		/* update shader projection matrix after updating its values in ram*/
		updateProjectionMatrix(projMatrix, deltaTime);
		glUseProgram(shader_program);
		glUniformMatrix4dv(matrix_location, 1, GL_FALSE, projMatrix);
		glBindVertexArray( vao );


  	// Draw points 0-3 from the currently bound VAO with current in-use shader.
  	glDrawArrays( GL_TRIANGLES, 0, 3 );
  
  	// Put the stuff we've been drawing onto the visible area.
  	glfwSwapBuffers( window );
	}

	glfwTerminate();

	return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
	if(action == GLFW_PRESS){
		fprintf(stderr, "Key %d was Pressed\n", key);
	}
	if(action == GLFW_RELEASE){
		fprintf(stderr, "Key %d was Released\n", key);
	}
	//TODO:
}

/* update matrix based on time since last update */
void updateProjectionMatrix(GLdouble *matrix, double currTime){
	
	//rotate around y (up) axis
	matrix[0]		=  cos(currTime);
	matrix[2]		= -sin(currTime);
	matrix[8]		=  sin(currTime);
	matrix[10]	=  cos(currTime);
}