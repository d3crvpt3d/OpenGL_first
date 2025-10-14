#include "main.h"
#include "chunkMap.h"
#include <cstdint>
#include <filesystem>
#include <pthread.h>
#include <string.h>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "chunkGeneration.h"
#include "voxelTrace.h"
#include "stb_image.h"
#include "optimize_buffer.h"

#define ONLY_SPAWN_LOCATION 1

#if defined(_WIN32)
#include <windows.h>
#endif

std::string getRelativeRootDir(){

	char path[256];
#if defined(_WIN32)
	HMODULE hModule = GetModuleHandleA(NULL);
	GetModuleFileNameA(hModule, path, sizeof(path));
	char *lastBs = strrchr(path, '\\');
	if(lastBs != NULL){
		*lastBs = '\0';
	}
#else
	ssize_t count = readlink("/proc/self/exe", path, sizeof(path));
	path[count] = '\0';
	char *lastBs = strrchr(path, '/');
	if(lastBs != NULL){
		*lastBs = '\0';
	}
#endif
	return std::string(path);
}

Camera camera = {
	.xyz={0.0f, 0.0f, -1.0f},
	.f = 1.0f,
	.yaw_pitch={0.0f, 0.0f},
	.near_far={0.01f, 1000.0f},
	.aspectRatio=16.0f/9.0f,
};

double deltaTime;
double title_cd = 0.1; //Update title only every 100ms (if changed change reset value in main loop)
double xpos_old, ypos_old;
GLint nonFreqLocations[4];


char *loadShaders(const char* relative_path);
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
	//glfwWindowHint(GLFW_SAMPLES, 4);
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
	
	const GLfloat cube_vertecies[] = {
		
		//positions			//normals			 //texCoord
		0.f, 0.f, 0.f, -1.f, 0.f, 0.f, 0.0625f,	0.5f,		//right bot	//side
		0.f, 0.f, 1.f, -1.f, 0.f, 0.f, 0.f,			0.5f,		//left bot
		0.f, 1.f, 0.f, -1.f, 0.f, 0.f, 0.0625f,	0.25f,	//right top
		0.f, 1.f, 1.f, -1.f, 0.f, 0.f, 0.f,			0.25f,	//left top
		
		1.f, 0.f, 0.f,	1.f, 0.f, 0.f, 0.f,			0.5f,		//left bot	//side
		1.f, 0.f, 1.f,	1.f, 0.f, 0.f, 0.0625f,	0.5f,		//right bot
		1.f, 1.f, 0.f,	1.f, 0.f, 0.f, 0.f,			0.25f,	//left top
		1.f, 1.f, 1.f,	1.f, 0.f, 0.f, 0.0625f,	0.25f,	//right top

		0.f, 0.f, 0.f,	0.f,-1.f, 0.f, 0.f,			0.5f,		//left top	//bot
		0.f, 0.f, 1.f,	0.f,-1.f, 0.f, 0.f,			0.75f,	//left bot
		1.f, 0.f, 0.f,	0.f,-1.f, 0.f, 0.0625f,	0.5f,		//right top
		1.f, 0.f, 1.f,	0.f,-1.f, 0.f, 0.0625f,	0.75f,	//right bot
		
		0.f, 1.f, 0.f,	0.f, 1.f, 0.f, 0.f,			0.25f,	//left bot	//top
		0.f, 1.f, 1.f,	0.f, 1.f, 0.f, 0.f,			0.f,		//left top
		1.f, 1.f, 0.f,	0.f, 1.f, 0.f, 0.0625f,	0.25f,	//right bot
		1.f, 1.f, 1.f,	0.f, 1.f, 0.f, 0.0625f,	0.f,		//right top
		
		0.f, 0.f, 0.f,	0.f, 0.f,-1.f, 0.f,			0.5f,		//left bot	//side
		0.f, 1.f, 0.f,	0.f, 0.f,-1.f, 0.f,			0.25f,	//left top
		1.f, 0.f, 0.f,	0.f, 0.f,-1.f, 0.0625f,	0.5f,		//right bot
		1.f, 1.f, 0.f,	0.f, 0.f,-1.f, 0.0625f,	0.25f,	//right top
		
		0.f, 0.f, 1.f,	0.f, 0.f, 1.f, 0.0625f,	0.5f,		//right bot	//side
		0.f, 1.f, 1.f,	0.f, 0.f, 1.f, 0.0625f,	0.25f,	//right top
		1.f, 0.f, 1.f,	0.f, 0.f, 1.f, 0.f,			0.5f,		//left bot
		1.f, 1.f, 1.f,	0.f, 0.f, 1.f, 0.f,			0.25f,	//left top
	};
	
	const GLuint cube_index[] = {
		0,	3,	1,	0,	2,	3,
		4,	5,	7,	4,	7,	6,
		8,	9,	10, 9,	11,	10,
		12,	14,	13,	14,	15,	13,
		16,	18,	19,	16,	19,	17,
		20,	23,	22,	20,	21,	23
	};
	
	GLuint cubeVBO, cubeEAO;
	
	//create cubeVBO
	glGenBuffers(1, &cubeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER,
			sizeof(cube_vertecies),
			cube_vertecies,
			GL_STATIC_DRAW);
	
	//create cubeEAO
	glGenBuffers(1, &cubeEAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEAO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			sizeof(cube_index),
			cube_index,
			GL_STATIC_DRAW);

	
	/* Load Shaders */
	const char *vertex_shader = loadShaders("../shaders/vertex.glsl");
	const char *fragment_shader = loadShaders("../shaders/fragment.glsl");
	
	if(!vertex_shader || !fragment_shader){
		fprintf(stderr, "vertex shader or fragment shader not locatable\n");
		return -1;
	}
	
	/* OpenGL Options */
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
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
	

	//create VAO/VBO buffer map
	GLuint blocksVAO, blocksVBO;
	
	//gnerate VAOs at raw[0]
	glGenVertexArrays(1, &blocksVAO);
	
	glGenBuffers(1, &blocksVBO);

	//bind blocks vertex array
	glBindVertexArray(blocksVAO);


	//bind cubeVBO
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glVertexAttribPointer(0,
			3,
			GL_FLOAT,
			GL_FALSE,
			8 * sizeof(GLfloat),
			(void *) 0); //pos
	glVertexAttribPointer(1,
			3,
			GL_FLOAT,
			GL_FALSE,
			8 * sizeof(GLfloat),
			(void *) (sizeof(GLfloat) * 3)); //normal
	glVertexAttribPointer(2,
			2,
			GL_FLOAT,
			GL_FALSE,
			8 * sizeof(GLfloat),
			(void *) (sizeof(GLfloat) * 6)); //texcoord

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);


	//bind blocksVBO
	glBindBuffer(GL_ARRAY_BUFFER, blocksVBO);
	glVertexAttribIPointer(3,
			1,
			GL_UNSIGNED_INT,
			sizeof(BlockGPU_t),
			(void*) offsetof(BlockGPU_t, type));
	glVertexAttribDivisor(3, 1); //increase by one for each instance

	glVertexAttribPointer(4,
			3,
			GL_FLOAT,
			GL_FALSE,
			sizeof(BlockGPU_t),
			(void*) offsetof(BlockGPU_t, xyz));
	glVertexAttribDivisor(4, 1); //increase by one for each instance

	glEnableVertexAttribArray(3);//block type
	glEnableVertexAttribArray(4);//block pos

	//tell VAO the Element Array Buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEAO);

	glBindVertexArray(0);



	//use shader program
	glUseProgram(shader_program);

	//load textures
	int texWidth, texHeight, texNrChannels;
	uint32_t textureAtlas;
	glGenTextures(1, &textureAtlas);
	glActiveTexture(GL_TEXTURE0); //active texture slot to bind texture to
	glBindTexture(GL_TEXTURE_2D, textureAtlas);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//relative texData path
	std::filesystem::path exeRoot = getRelativeRootDir();
	std::filesystem::path img_path = exeRoot / "../texData" / "firstGLAtlats.png";
	uint8_t *texData = stbi_load(img_path.u8string().c_str(),
			&texWidth,
			&texHeight,
			&texNrChannels,
			0);
	if(!texData){
		fprintf(stderr, "Could not load image\n");
	}
	glTexImage2D(GL_TEXTURE_2D,
			0,
			GL_RGB,
			texWidth,
			texHeight,
			0,
			GL_RGB,
			GL_UNSIGNED_BYTE,
			texData);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(texData);
	
	glUniform1i(glGetUniformLocation(shader_program, "aTexture"), 0);

	//end load textures

	
	//opengl state changes
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	
	glfwSetCursorPosCallback(window, cursor_callback);
	
	//mouse position
	glfwGetCursorPos(window, &xpos_old, &ypos_old);
	
	vec3i_t lastChunk = {0, 0, 0};
	
	setUpThreads();
	
	generateSpawnLocation();

	vec3i_t break_block = {0, 0, 0};
	vec3i_t place_block = {0, 0, 0};

	ssize_t numInstances = 0;
	
	/* MAIN LOOP */
	/* MAIN LOOP */
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
		
		//push chunk data to non-active blockVBO
		if(!genChunksQueue.empty()){
			
			pthread_mutex_lock(&genChunksQueue_mutex);
			std::vector<BlockGPU_t> optimized_buffer_data;

			while(!genChunksQueue.empty()){

				//get all chunks in queue
				vec3i_t currCoords = genChunksQueue.front();
				genChunksQueue.pop();

				int32_t x = currCoords.x;
				int32_t y = currCoords.y;
				int32_t z = currCoords.z;

				//get RAM data
				Chunk_t *chunk = chunkMap_get(chunkMap, x, y, z);

				//append optimized Data for VRAM
				std::vector<BlockGPU_t> currentblocks = gen_optimized_buffer(*chunk);
				
				//add to number of instances
				numInstances += currentblocks.size();

				optimized_buffer_data.insert(
						optimized_buffer_data.end(),
						currentblocks.begin(),
						currentblocks.end()) ;
			}
			//unlock queue
			pthread_mutex_unlock(&genChunksQueue_mutex);

			//bind Chunks VAO
			glBindVertexArray(blocksVAO);

			//overwrite VAOs VBO with implicit shadow buffer
			glBindBuffer(
					GL_ARRAY_BUFFER,
					blocksVBO);
			
			glBufferData(
					GL_ARRAY_BUFFER,
					sizeof(BlockGPU_t) * optimized_buffer_data.size(),
					optimized_buffer_data.data(),
					GL_STATIC_DRAW);

			//unbind current Chunks VAO
			glBindVertexArray(0);

			numInstances = optimized_buffer_data.size();
		}
		
		handle_keys(window);
		
		//update place/break block coordinates
		update_lookingAt(camera.xyz,
				camera.yaw_pitch,
				&break_block,
				&place_block,
				BLOCK_RANGE);

		// update Uniforms
		glUniform3fv(campos_loc, 1, camera.xyz);
		glUniform2fv(camdir_loc, 1, camera.yaw_pitch);
		
		
		//check if new chunk
		currChunk.x = ((int32_t) floorf(camera.xyz[0])) / 64;
		currChunk.y = ((int32_t) floorf(camera.xyz[1])) / 64;
		currChunk.z = ((int32_t) floorf(camera.xyz[2])) / 64;
		
		if(!ONLY_SPAWN_LOCATION){
			if(
					currChunk.x != lastChunk.x ||
					currChunk.y != lastChunk.y ||
					currChunk.z != lastChunk.z
			  ){
				//TODO: check
				//fprintf(stderr,"Current Chunk:%d,%d,%d\n", currChunk.x, currChunk.y, currChunk.z); //DEBUG
				//fprintf(stderr, "Pos: %f, %f, %f\n", camera.xyz[0], camera.xyz[1], camera.xyz[2]);
				addNewChunkJobs(lastChunk.x,
						lastChunk.y,
						lastChunk.z,
						currChunk.x,
						currChunk.y,
						currChunk.z);
				lastChunk.x = currChunk.x;
				lastChunk.y = currChunk.y;
				lastChunk.z = currChunk.z;
			}
		}

		glUseProgram(shader_program);
		
		glBindVertexArray(blocksVAO);

		glDrawElementsInstanced(GL_TRIANGLES,
				36,
				GL_UNSIGNED_INT,
				0,
				numInstances);

		// Put the stuff we've been drawing onto the visible area.
		glfwSwapBuffers( window );
		
		// Update window events.
		glfwPollEvents();
		
	}
	
	chunkMap_destroy(chunkMap);
	
	glfwTerminate();
	
	free((void*) vertex_shader);
	free((void*) fragment_shader);
	
	return 0;
}

//load with relative path
char *loadShaders(const char* relative_path){

	std::filesystem::path exeRoot = getRelativeRootDir();
	std::filesystem::path new_path = exeRoot / relative_path;
	//open shader files
	FILE *fptr = fopen(new_path.u8string().c_str(), "rb");
	
	if(!fptr){
		fprintf(stderr, "%s does not exist\n", new_path.u8string().c_str());
		return NULL;
	}
	
	fseek(fptr, 0, SEEK_END);
	uint64_t size = ftell(fptr);
	fseek (fptr, 0, SEEK_SET);
	
	char *buffer = (char *) malloc(size + 1);
	
	ssize_t num_read = fread(buffer, 1, size, fptr);
	
	buffer[size] = '\0';
	
	fclose(fptr);
	return buffer;
}


void handle_keys(GLFWwindow *window){
	
	static uint8_t esc_down = 0;
	static uint8_t f_r_near_far_change = 0xFF; //if focal length, aspect-ratio, near or far changed
	static uint32_t flyspeed = FLYSPEED;
	
	//movement
	if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
		camera.xyz[0] += deltaTime * flyspeed * -sin(-camera.yaw_pitch[0]);
		camera.xyz[2] += deltaTime * flyspeed * cos(-camera.yaw_pitch[0]);
	}
	if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
		camera.xyz[0] += deltaTime * flyspeed * sin(-camera.yaw_pitch[0]);
		camera.xyz[2] += deltaTime * flyspeed * -cos(-camera.yaw_pitch[0]);
	}
	
	if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
		camera.xyz[0] += deltaTime * flyspeed * -cos(-camera.yaw_pitch[0]);
		camera.xyz[2] += deltaTime * flyspeed * -sin(-camera.yaw_pitch[0]);
	}
	if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
		camera.xyz[0] += deltaTime * flyspeed * cos(-camera.yaw_pitch[0]);
		camera.xyz[2] += deltaTime * flyspeed * sin(-camera.yaw_pitch[0]);
	}
	
	if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
		camera.xyz[1] += deltaTime * flyspeed;
	}
	if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
		camera.xyz[1] -= deltaTime * flyspeed;
	}
	
	if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS){
		flyspeed = FLYSPEED_MAX;
	}else{
		flyspeed = FLYSPEED;
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

	//Wireframe Toggle
	static bool f3_down = false;
	static bool wireframe_active = false;
	if(glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS && !f3_down){
		f3_down = true;

		if(wireframe_active){
			wireframe_active = false;
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}else{
			wireframe_active = true;
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}

	}
	if(glfwGetKey(window, GLFW_KEY_F3) == GLFW_RELEASE){
		f3_down = false;
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

void generateSpawnLocation(){
	for(int16_t z = -RENDERDISTANCE; z <= RENDERDISTANCE; z++){
		for(int16_t y = -RENDERDISTANCE; y <= RENDERDISTANCE; y++){
			for(int16_t x = -RENDERDISTANCE; x <= RENDERDISTANCE; x++){
				addJob(x, y, z);
			}
		}
	}
}
