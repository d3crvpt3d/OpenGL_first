#include "glad/gl.h"
#include "main.h"
#include <GLFW/glfw3.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define WIDTH 	1920
#define HEIGHT 	1080
#define FLYSPEED 1.5
#define RADPERPIXEL 0.00418879f //mouse sensitivity
#define PI 3.1415927f

char *loadShaders(const char* path);

int main(){

	GLenum err;

	GLFWwindow *window;
	
	if(!glfwInit()){
		return -1;
	}

	/* GLFW Window Hints */
	glfwWindowHint(GLFW_SAMPLES, 4);
	//glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER , GLFW_TRUE);
  //glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); 

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
   -0.5f, -0.5f,  0.0f,
   	0.5f, -0.5f,  0.0f,
	};

	float colors[] = {
  	1.0f,  0.0f,  0.0f,
  	0.0f,  1.0f,  0.0f,
  	0.0f,  0.0f,  1.0f
	};

	float normals[] = {
  	0.0f,  0.0f, -1.0f,
  	0.0f,  0.0f, -1.0f,
  	0.0f,  0.0f, -1.0f
	};
	
	Camera camera = {
		.x=0.0f, .y=0.0f, .z=-1.0f,
		.pitch=0.0f, .yaw=0.0f,
		.far=1000.0f, .near=0.001f,
		.aspectRatio=16.0f/9.0f,
		.f = 1
	};

	//TODO: change to dynamic lights
	Light lights[1] = {
		0.0, 10.0, 2.0,//xyz
		0.0, -10.0/sqrt(104.0), -2.0/sqrt(104.0),//direction (normal)
		1.0, 1.0, 1.0,//color
		0.8						//intensity 
	};

	GLuint points_vbo = 0;
	glGenBuffers( 1, &points_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, points_vbo );
	glBufferData( GL_ARRAY_BUFFER, 9 * sizeof( float ), points, GL_STATIC_DRAW );
	
	GLuint colors_vbo = 0;
	glGenBuffers( 1, &colors_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, colors_vbo );
	glBufferData( GL_ARRAY_BUFFER, 9 * sizeof( float ), colors, GL_STATIC_DRAW );

	
	GLuint normals_vbo = 0;
	glGenBuffers( 1, &normals_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, normals_vbo );
	glBufferData( GL_ARRAY_BUFFER, 9 * sizeof( float ), normals, GL_STATIC_DRAW );


	/* create VAO */
	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(2);


	/* Load Shaders */

	const char *vertex_shader = loadShaders("shaders/vertex.glsl");
	const char *fragment_shader = loadShaders("shaders/fragment.glsl");

	if(!vertex_shader || !fragment_shader){
		fprintf(stderr, "vertex shader or fragment shader not locatable\n");
		return -1;
	}

  /* OpenGL Options */
  glEnable(GL_CULL_FACE);

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

	/* Check Compilation Errors */
	GLint success;
  glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
  if (!success) {
      char infoLog[512];
      glGetShaderInfoLog(fs, 512, NULL, infoLog);
      fprintf(stderr, "Fragment shader compilation failed: %s\n", infoLog);
      free((void*)fragment_shader);
      return -1;
  }

  glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
  if (!success) {
      char infoLog[512];
    	glGetShaderInfoLog(vs, 512, NULL, infoLog);
      fprintf(stderr, "Vertex shader compilation failed: %s\n", infoLog);
      free((void*)vertex_shader);
      return -1;
    }

	//set up Input callbacks

	double currTime = glfwGetTime();
	double lastTime;
	double deltaTime;

	/* get location of matrix in shader */
	GLint cameraPosLocation = glGetUniformLocation(shader_program, "cameraPos");
	GLint yaw_pitchLocation = glGetUniformLocation(shader_program, "yaw_pitch");
	GLint nearfar_location = glGetUniformLocation(shader_program, "near_far");
	GLint f_location = glGetUniformLocation(shader_program, "f");
	GLint ratio_location = glGetUniformLocation(shader_program, "ratio");
	GLint light_location = glGetUniformLocation(shader_program, "LightPosition");
	GLint lColor_location = glGetUniformLocation(shader_program, "LightColor");
	GLint lI_location = glGetUniformLocation(shader_program, "LightIntensity");
	GLint lDir_location = glGetUniformLocation(shader_program, "LightDirection");

	glUseProgram(shader_program);
	
	//for once use static light
	glUniform3f(light_location, lights[0].x, lights[0].y, lights[0].z);
	glUniform3fv(lColor_location, 3, lights[0].rgb);
	glUniform1f(lI_location, lights[0].intensity);
	glUniform3fv(lDir_location, 3, lights[0].direction);

	double title_cd = 0.1; //Update title only every 100ms (if changed change reset value in main loop)
	uint8_t esc_down = 0;
	
	//mouse position
	double xpos, ypos;
	double xpos_old = 0, ypos_old = 0;
	
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
			title_cd = 0.1; //reset value of title cd
		}

  	// Wipe the drawing surface clear.
  	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		//camera direction
		glfwGetCursorPos(window, &xpos, &ypos);
		camera.yaw 		+= (xpos - xpos_old) * RADPERPIXEL;
		camera.pitch 	+= (ypos - ypos_old) * RADPERPIXEL;
		xpos_old = xpos;
		ypos_old = ypos;

		/* clamp pitch to 180° */
		if(camera.pitch > PI){
			camera.pitch = PI;
		}
		if(camera.pitch < -PI){
			camera.pitch = -PI;
		}

		
		// update Uniforms
		glUniform3f(cameraPosLocation,	camera.x, 		camera.y,	camera.z);
		glUniform2f(yaw_pitchLocation,	camera.yaw, 	camera.pitch);
		glUniform2f(nearfar_location, 	camera.near,	camera.far);
		glUniform1f(f_location, 				camera.f);
		glUniform1f(ratio_location, 		camera.aspectRatio);


		//render scene
		glBindVertexArray( vao );
		glDrawArrays( GL_TRIANGLES, 0, 3 );
  
  	// Put the stuff we've been drawing onto the visible area.
  	glfwSwapBuffers( window );
		
		// Update window events.
  	glfwPollEvents();

    /* USER INPUT */

		//movement //TODO: fix
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
      camera.x += deltaTime * FLYSPEED * -sin(-camera.yaw);
			camera.z += deltaTime * FLYSPEED * cos(-camera.yaw);
		}
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
      camera.x += deltaTime * FLYSPEED * sin(-camera.yaw);
			camera.z += deltaTime * FLYSPEED * -cos(-camera.yaw);
    }
		
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
			camera.x += deltaTime * FLYSPEED * -cos(-camera.yaw);
			camera.z += deltaTime * FLYSPEED * -sin(-camera.yaw);
		}
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
			camera.x += deltaTime * FLYSPEED * cos(-camera.yaw);
			camera.z += deltaTime * FLYSPEED * sin(-camera.yaw);
		}

    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
      camera.y += deltaTime * FLYSPEED;
    }
    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
      camera.y -= deltaTime * FLYSPEED;
    }
		
		// toggle mouse capture
		if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && !esc_down){
			esc_down = 1;
      glfwSetInputMode(window, GLFW_CURSOR, glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);//toggle cursor
			glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }
		if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE){
			esc_down = 0;
		}
	}

	glfwTerminate();

	free((void*)vertex_shader);
	free((void*)fragment_shader);

	return 0;
}


char *loadShaders(const char* path){
	FILE *fptr = fopen(path, "rb");

	if(!fptr){
		return NULL;
	}

	fseek(fptr, 0, SEEK_END);
	uint64_t size = ftell(fptr);
	fseek (fptr, 0, SEEK_SET);

	char *buffer = (char *) malloc(size + 1);

	fread(buffer, 1, size, fptr);
	
	buffer[size] = '\0';

	fclose(fptr);
	return buffer;
}