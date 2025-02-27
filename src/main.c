#include "main.h"

Camera camera = {
	.xyz={0.0f, 0.0f, -1.0f},
	.yaw_pitch={0.0f, 0.0f},
	.near_far={0.01f, 1000.0f},
	.aspectRatio=16.0f/9.0f,
	.f = 1.0f
};

double deltaTime;
double title_cd = 0.1; //Update title only every 100ms (if changed change reset value in main loop)
double xpos_old, ypos_old;
GLint nonFreqLocations[4];


char *loadShaders(const char* path);
void cursor_callback(GLFWwindow *window, double xpos, double ypos);
void handle_keys(GLFWwindow *window);


void updateNonFreq(Camera *cam, uint8_t *m, GLint *locations){
	uint8_t mask = *m;
	
	//f
	if(mask & 0x80){
		mask ^= 0x80;
		glUniform1f(locations[0], cam->f);
	}
	//ratio
	if(mask & 0x40){
		mask ^= 0x40;
		glUniform1f(locations[1], cam->aspectRatio);
	}
	//near
	if(mask & 0x20){
		mask ^= 0x20;
		glUniform1f(locations[2], cam->near_far[0]);
		glUniform1f(locations[3], cam->near_far[1]);
	}
	//far
	if(mask & 0x10){
		mask ^= 0x10;
	}
	if(mask & 0x10){
		mask ^= 0x10;
	}
	
	*m = mask;
}

int main(){
	GLenum err;
	
	GLFWwindow *window;
	
	if(!glfwInit()){
		return -1;
	}
	
	/* GLFW Window Hints */
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_DEPTH_BITS, 24);
	//glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER , GLFW_TRUE);
	//glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); 
	
	/* Create GLFW window */
	window = glfwCreateWindow(WIDTH, HEIGHT, "Test Title!", NULL, NULL);
	
	if(!window){
		glfwTerminate();
		return -1;
	}
	
	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_CURSOR,GLFW_CURSOR_DISABLED);//toggle cursor
	
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
	

	static const GLfloat cube_strip[] = {
    -1.f, 1.f, 1.f,     // Front-top-left
    1.f, 1.f, 1.f,      // Front-top-right
    -1.f, -1.f, 1.f,    // Front-bottom-left
    1.f, -1.f, 1.f,     // Front-bottom-right
    1.f, -1.f, -1.f,    // Back-bottom-right
    1.f, 1.f, 1.f,      // Front-top-right
    1.f, 1.f, -1.f,     // Back-top-right
    -1.f, 1.f, 1.f,     // Front-top-left
    -1.f, 1.f, -1.f,    // Back-top-left
    -1.f, -1.f, 1.f,    // Front-bottom-left
    -1.f, -1.f, -1.f,   // Back-bottom-left
    1.f, -1.f, -1.f,    // Back-bottom-right
    -1.f, 1.f, -1.f,    // Back-top-left
    1.f, 1.f, -1.f      // Back-top-right
	};

	//set up instance vbo of cubes
	GLint cubeVAO;
	glGenVertexArrays(1, &cubeVAO);
	glBindVertexArray(cubeVAO);


	//create cubeVBO
	GLint cubeVBO;
	glGenBuffers(1, &cubeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_strip), cube_strip, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
	glEnableVertexAttribArray(0);


	/* Load Shaders */
	const char *vertex_shader = loadShaders("E:/Code/Projects/OpenGL/opengl_glfw_1/shaders/vertex.glsl");
	const char *fragment_shader = loadShaders("E:/Code/Projects/OpenGL/opengl_glfw_1/shaders/fragment.glsl");
	
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
	
	double currTime = glfwGetTime();
	double lastTime;
	
	GLint campos_loc = glGetUniformLocation(shader_program, "cam_pos");
	GLint camdir_loc = glGetUniformLocation(shader_program, "cam_dir");
	
	GLint faces_normal_loc = glGetUniformLocation(shader_program, "face_normal");
	
	nonFreqLocations[0] = glGetUniformLocation(shader_program, "f");
	nonFreqLocations[1] = glGetUniformLocation(shader_program, "ratio");
	nonFreqLocations[2] = glGetUniformLocation(shader_program, "near");
	nonFreqLocations[3] = glGetUniformLocation(shader_program, "far");
	
	glUseProgram(shader_program);
	
	//opengl state changes
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	
	glfwSetCursorPosCallback(window, cursor_callback);
	
	//mouse position
	glfwGetCursorPos(window, &xpos_old, &ypos_old);
	
	vec3i_t lastChunk = {0.0, 0.0, 0.0};

	gchunkGenThread = generateChunks(lastChunk);

	/* MAIN LOOP */
	while ( !glfwWindowShouldClose( window ) ) {

		// Wipe the drawing surface clear.
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		
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
		
		handle_keys(window);
		
		// update Uniforms
		glUniform3fv(campos_loc, 1, camera.xyz);
		glUniform2fv(camdir_loc, 1, camera.yaw_pitch);
		
		
		/* create chunks async */
		vec3i_t currChunk = {camera.xyz[0] / CHUNK_WD, camera.xyz[1] / CHUNK_H, camera.xyz[2] / CHUNK_WD};
		
		//check if new chunk
		if(
			currChunk.x != lastChunk.x ||
			currChunk.y != lastChunk.y ||
			currChunk.z != lastChunk.z
		){
			lastChunk.x = currChunk.x;
			lastChunk.y = currChunk.y;
			lastChunk.z = currChunk.z;

			if(gthreadDone){
				pthread_join(gchunkGenThread, NULL);
			}else{
				gthreadDone = 0;
				gchunkGenThread = generateChunks(currChunk);
			}
		}
		
		glUseProgram(shader_program);
		glBindVertexArray(cubeVAO);

		//TODO: draw cubes instanced
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 42);
		
		// Put the stuff we've been drawing onto the visible area.
		glfwSwapBuffers( window );
		
		// Update window events.
		glfwPollEvents();
		
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


void handle_keys(GLFWwindow *window){
	
	static uint8_t esc_down = 0;
	static uint8_t f_r_near_far_change = 0xFF; //if focal length, aspect-ratio, near or far changed
	
	//movement
	if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
		camera.xyz[0] += deltaTime * FLYSPEED * -sin(-camera.yaw_pitch[0]);
		camera.xyz[2] += deltaTime * FLYSPEED * cos(-camera.yaw_pitch[0]);
	}
	if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
		camera.xyz[0] += deltaTime * FLYSPEED * sin(-camera.yaw_pitch[0]);
		camera.xyz[2] += deltaTime * FLYSPEED * -cos(-camera.yaw_pitch[0]);
	}
	
	if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
		camera.xyz[0] += deltaTime * FLYSPEED * -cos(-camera.yaw_pitch[0]);
		camera.xyz[2] += deltaTime * FLYSPEED * -sin(-camera.yaw_pitch[0]);
	}
	if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
		camera.xyz[0] += deltaTime * FLYSPEED * cos(-camera.yaw_pitch[0]);
		camera.xyz[2] += deltaTime * FLYSPEED * sin(-camera.yaw_pitch[0]);
	}
	
	if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
		camera.xyz[1] += deltaTime * FLYSPEED;
	}
	if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
		camera.xyz[1] -= deltaTime * FLYSPEED;
	}
	
	//zoom
	if(glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS){
		if(camera.f != 2.0f){
			camera.f = 2.0f;
			f_r_near_far_change |= 0x80;
		}
	}else{
		if(camera.f != 1.0f){
			camera.f = 1.0f;
			f_r_near_far_change |= 0x80;
		}
	}
	
	//check if (f r near far) changed
	updateNonFreq(&camera, &f_r_near_far_change, nonFreqLocations);
	
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

//mouse callback
void cursor_callback(GLFWwindow *window, double xpos, double ypos){
	
	glfwGetCursorPos(window, &xpos, &ypos);
	camera.yaw_pitch[0] += (xpos - xpos_old) * RADPERPIXEL;
	camera.yaw_pitch[1] += (ypos - ypos_old) * RADPERPIXEL;
	xpos_old = xpos;
	ypos_old = ypos;
	
	/* clamp pitch to 180° */
	if(camera.yaw_pitch[1] > PI/2){
		camera.yaw_pitch[1] = PI/2;
	}
	if(camera.yaw_pitch[1] < -PI/2){
		camera.yaw_pitch[1] = -PI/2;
	}
}