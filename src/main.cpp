#include <array>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <mutex>
#include <pthread.h>
#include <string.h>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION


//#define TEXTURE_PATH "texData/firstGLAtlats.png"
#define TEXTURE_PATH "texData/faithful_32.png"

#define CHUNK_UPLOAD_PER_FRAME 10


#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "main.h"
#include "chunkMap.h"
#include "chunkGeneration.h"
#include "voxelTrace.h"
#include "stb_image.h"
#include "optimize_buffer.h"
#include "frustumCulling.h"
#include "bufferMap.h"


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
	.grace_space=PI-PI/(2.0*1.79),
};

double deltaTime;
double title_cd = 0.1; //Update title only every 100ms (if changed change reset value in main loop)
double xpos_old, ypos_old;
GLint nonFreqLocations[4];

char *loadShaders(const char* relative_path);
void cursor_callback(GLFWwindow *window, double xpos, double ypos);
void handle_keys(GLFWwindow *window);

//for updateVramThread to have access
void* mapped_regions[2]; //persistend mapped pointers
std::atomic<int> current_buffer{0};
std::atomic<int> instance_count_perBuffer[2][6]{0};

struct BaseVertex {
	GLfloat pos[3];
	GLfloat nml[3];
	GLfloat uv[2];
};

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
	glfwSwapInterval(0);
	
	int version_glad = gladLoadGL(glfwGetProcAddress);
	if ( version_glad == 0 ) {
		fprintf( stderr, "ERROR: Failed to initialize OpenGL context.\n" );
		return 1;
	}
	printf( "Loaded OpenGL %i.%i\n", GLAD_VERSION_MAJOR( version_glad ), GLAD_VERSION_MINOR( version_glad ) );
	printf( "Renderer: %s.\n", glGetString( GL_RENDERER ) );
	printf( "OpenGL version supported %s.\n", glGetString( GL_VERSION ) );
	
	const BaseVertex baseMeshes[24] = {
	// 0: -X Face (Links)
    { {0,0,0}, {-1,0,0}, {0.0625,0.5} },
    { {0,1,0}, {-1,0,0}, {0.0625,0.25} },
    { {0,0,1}, {-1,0,0}, {0,0.5} },
    { {0,1,1}, {-1,0,0}, {0,0.25} },
    
    // 1: +X Face (Rechts)
    { {1,0,1}, { 1,0,0}, {0.0625,0.5} },
    { {1,1,1}, { 1,0,0}, {0.0625,0.25} },
    { {1,0,0}, { 1,0,0}, {0,0.5} },
    { {1,1,0}, { 1,0,0}, {0,0.25} },

    // 2: -Y Face (Unten)
    { {0,0,0}, { 0,-1,0}, {0.0625,0.75} },
    { {0,0,1}, { 0,-1,0}, {0.0625,0.5} },
    { {1,0,0}, { 0,-1,0}, {0,0.75} },
    { {1,0,1}, { 0,-1,0}, {0,0.5} },

    // 3: +Y Face (Oben)
    { {1,1,0}, { 0, 1,0}, {0.0625,0.25} },
    { {1,1,1}, { 0, 1,0}, {0.0625,0} },
    { {0,1,0}, { 0, 1,0}, {0,0.25} },
    { {0,1,1}, { 0, 1,0}, {0,0} },

    // 4: -Z Face (Vorne)
    { {1,0,0}, { 0,0,-1}, {0.0625,0.5} },
    { {1,1,0}, { 0,0,-1}, {0.0625,0.25} },
    { {0,0,0}, { 0,0,-1}, {0,0.5} },
    { {0,1,0}, { 0,0,-1}, {0,0.25} },

    // 5: +Z Face (Hinten)
    { {0,0,1}, { 0,0, 1}, {0.0625,0.5} },
    { {0,1,1}, { 0,0, 1}, {0.0625,0.25} },
    { {1,0,1}, { 0,0, 1}, {0,0.5} },
    { {1,1,1}, { 0,0, 1}, {0,0.25} }
	};
	
	const GLuint face_index[] = {
		0, 1, 2, 3
	};
	
	GLuint instanceVAO, faceVBO, faceEAO;

	glGenVertexArrays(1, &instanceVAO);

	glBindVertexArray(instanceVAO);
	
	//create cubeVBO
	glGenBuffers(1, &faceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, faceVBO);
	glBufferData(GL_ARRAY_BUFFER,
			sizeof(baseMeshes),
			baseMeshes,
			GL_STATIC_DRAW);
	
	//create faceEAO
	glGenBuffers(1, &faceEAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faceEAO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			sizeof(face_index),
			face_index,
			GL_STATIC_DRAW);

	//aVertexPosition
	glVertexAttribFormat(0,
			3,
			GL_FLOAT,
			GL_FALSE,
			offsetof(BaseVertex, pos));
	glVertexAttribBinding(0, 0);
	glEnableVertexAttribArray(0);

	//aNormal
	glVertexAttribFormat(1,
			3,
			GL_FLOAT,
			GL_FALSE,
			offsetof(BaseVertex, nml));
	glVertexAttribBinding(1, 0);
	glEnableVertexAttribArray(1);

	//aTexCoord
	glVertexAttribFormat(2,
			2,
			GL_FLOAT,
			GL_FALSE,
			offsetof(BaseVertex, uv));
	glVertexAttribBinding(2, 0);
	glEnableVertexAttribArray(2);

	//bind mesh VBO to binding idx 0
	glBindVertexBuffer(0, faceVBO, 0, sizeof(BaseVertex));
	
	/* Load Shaders */
	const char *vertex_shader = loadShaders("../shaders/vertex.glsl");
	const char *fragment_shader = loadShaders("../shaders/fragment.glsl");
	
	if(!vertex_shader || !fragment_shader){
		fprintf(stderr, "vertex shader or fragment shader not locatable\n");
		return -1;
	}
	
	/* OpenGL Options */
	//left hand coordinate system
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
	
	GLint face_loc = glGetUniformLocation(shader_program, "face");
	GLint projMatrix_loc = glGetUniformLocation(shader_program, "projMatrix");
	GLint camPos_loc = glGetUniformLocation(shader_program, "camPos");
	

	//gen instance VBOs
	std::array<GLuint, CHUNKS> tmpIDS;
	glGenBuffers(CHUNKS, tmpIDS.data());

	//copy instanceVBO IDs to data structure
	uint64_t idx = 0;
	for(uint32_t z = 0; z < RENDERSPAN; z++){
		for(uint32_t y = 0; y < RENDERSPAN; y++){
			for(uint32_t x = 0; x < RENDERSPAN; x++){
				ChunkCPU_t *cChunk = bufferMap.at(x, y, z);
				cChunk->instanceVBO = tmpIDS[idx++];
			}
		}
	}

	//pos -> size -> type
	glVertexAttribFormat(3, 3,
			GL_FLOAT,
			GL_FALSE,
			offsetof(QuadGPU_t, xyz));
	glVertexAttribBinding(3, 1);
	glEnableVertexAttribArray(3);

	glVertexAttribIFormat(4, 2,
			GL_UNSIGNED_INT,
			offsetof(QuadGPU_t, size));
	glVertexAttribBinding(4, 1);
	glEnableVertexAttribArray(4);

	glVertexAttribIFormat(5, 1,
			GL_UNSIGNED_SHORT,
			offsetof(QuadGPU_t, type));
	glVertexAttribBinding(5, 1);
	glEnableVertexAttribArray(5);

	glVertexBindingDivisor(1, 1);

	//bind CubeEAO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faceEAO);

	//dummy instanceVBO
	glBindVertexBuffer(1, 0, 0, sizeof(QuadGPU_t));

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
	std::filesystem::path img_path = exeRoot / "../" / TEXTURE_PATH;
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
	
	currChunk.x = 0;
	currChunk.y = 0;
	currChunk.z = 0;
	
	setUpThreads();

	vec3i_t break_block = {0, 0, 0};
	vec3i_t place_block = {0, 0, 0};

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
			char tmp[32];
			snprintf(tmp, sizeof(tmp), "FPS: %.2lf", fps);
			glfwSetWindowTitle(window, tmp);
			title_cd = 0.1; //reset value of title cd
		}

		handle_keys(window);


		//update transformation matrix
		GLfloat a = cos(camera.yaw_pitch[0]);
		GLfloat b = sin(camera.yaw_pitch[0]);
		GLfloat c = cos(camera.yaw_pitch[1]);
		GLfloat j = sin(camera.yaw_pitch[1]);
		GLfloat e = camera.aspectRatio;
		GLfloat f = camera.f;
		GLfloat g = camera.near_far[1];
		GLfloat h = camera.near_far[0];

		//wolfram alpha regelt
		GLfloat projMatrix[16] = {
			a*f, 0, -b*f, 0,
			e*b*f*j, e*c*f, e*a*f*j, 0,
			b*c*(g+h)/(g-h), -j*(g+h)/(g-h), a*c*(g+h)/(g-h), -2*g*h/(g-h),
			b*c, -j, a*c, 0
		};

		glUniformMatrix4fv(projMatrix_loc, 1, GL_TRUE, projMatrix);

		//add camera pos (could be included in projMat)
		glUniform3fv(camPos_loc, 1, camera.xyz);


		//update place/break block coordinates
		update_lookingAt(camera.xyz,
				camera.yaw_pitch,
				&break_block,
				&place_block,
				BLOCK_RANGE);

		//each frame upload one VBO
		//orphaning if data already there
		//try_lock() for prob. 'speed'
		if(toUploadQueue_mutex.try_lock()){

			//upload a*buffers per frame
			for(uint32_t a = 0; a < CHUNK_UPLOAD_PER_FRAME; a++){
				if(!toUploadQueue.empty()){

					//uploadQueue has instance Data of chunk
					BufferCache_t q = std::move(toUploadQueue.front());
					toUploadQueue.pop();
					toUploadQueue_mutex.unlock();

					//get chunk that holds this chunks instanceVBO
					ChunkCPU_t *chunk = bufferMap.at(q.x, q.y, q.z);

					//skip if empty
					if(!chunk){
						continue;
					}

					//update coords (ringbuffer)
					chunk->x = q.x;
					chunk->y = q.y;
					chunk->z = q.z;

					//set count and offset of each side
					for(uint8_t side = 0; side < 6; side++){
						chunk->count[side] = q.size[side];
						chunk->offset[side] = q.offset[side];
					}


					//bind vbo of this chunk
					glBindBuffer(GL_ARRAY_BUFFER, chunk->instanceVBO);

					//Orphaning
					glBufferData(GL_ARRAY_BUFFER,
							q.data.size() * sizeof(QuadGPU_t),
							NULL,
							GL_DYNAMIC_DRAW);

					//upload data to this VBO
					if(!q.data.empty()){
						glBufferSubData(GL_ARRAY_BUFFER,
								0,
								q.data.size() * sizeof(QuadGPU_t),
								q.data.data());
					}

					//unbind for clean
					glBindBuffer(GL_ARRAY_BUFFER, 0);
				}else{
					//nothing to upload in queue
					toUploadQueue_mutex.unlock();
					break;
				}
			}//for loop end
		}//queue not lockable


		//DRAW of blocks
		//CPU side culling to skip sending backside of faces
		float cos_yaw = cos(camera.yaw_pitch[0]);
		float sin_yaw = sin(camera.yaw_pitch[0]);
		float cos_pit = cos(camera.yaw_pitch[1]);
		float sin_pit = sin(camera.yaw_pitch[1]);

		float cam_normal_x = sin_yaw * cos_pit;
		float cam_normal_y = -sin_pit;
		float cam_normal_z = cos_yaw * cos_pit;

		float theta_face[6];
		theta_face[0] = acos(cam_normal_x);
		theta_face[1] = acos(-cam_normal_x);
		theta_face[2] = acos(cam_normal_y);
		theta_face[3] = acos(-cam_normal_y);
		theta_face[4] = acos(cam_normal_z);
		theta_face[5] = acos(-cam_normal_z);
		//end CPU side culling to skip sending backside of faces

		//draw each face
		glUseProgram(shader_program);
		glBindVertexArray(instanceVAO);
		float grace_space = camera.grace_space;

		//for each chunk
		for(int64_t lz = currChunk.z - RENDERDISTANCE; lz <= currChunk.z + RENDERDISTANCE; lz++){
			for(int64_t ly = currChunk.y - RENDERDISTANCE; ly <= currChunk.y + RENDERDISTANCE; ly++){
				for(int64_t lx = currChunk.x - RENDERDISTANCE; lx <= currChunk.x + RENDERDISTANCE; lx++){

					ChunkCPU_t *chunkData = bufferMap.at(lx, ly, lz);

					if(chunkData->instanceVBO == 0
							|| chunkData->x != lx
							|| chunkData->y != ly
							|| chunkData->z != lz){
						continue;
					}

					//TODO:implement
					if(outOfFrustum(currChunk,
								lx, ly, lz,
								cam_normal_x,
								cam_normal_y,
								cam_normal_z)){
						continue;
					}

					for(uint8_t side = 0; side < 6; side++){

						//skip if empty side
						if(!chunkData->count[side]){
							continue;
						}

						if(theta_face[side] > grace_space){
							continue; //TODO:CHECK IF WORKING
						}

						glBindVertexBuffer(1,
								chunkData->instanceVBO,
								chunkData->offset[side],
								sizeof(QuadGPU_t));

						glDrawElementsInstancedBaseVertex(GL_TRIANGLE_STRIP,
								4,
								GL_UNSIGNED_INT,
								0,
								chunkData->count[side],
								side * 4);
					}

				}
			}
		}
		//ACTUAL DRAW END

		// Put the stuff we've been drawing onto the visible area.
		glfwSwapBuffers( window );
		
		// Update window events.
		glfwPollEvents();
		
	}
	
	//join children
	clearThreads();

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

inline int32_t worldToChunk(float worldPos){
	return (int32_t) floor(worldPos / 64.0f);
}

void handle_keys(GLFWwindow *window){
	
	static uint8_t esc_down = 0;
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
		if(camera.f != 4.0f){
			camera.f = 4.0f;
			camera.grace_space = PI-PI/(1.79*2.0*camera.f);
		}
	}else{
		if(camera.f != 1.0f){
			camera.f = 1.0f;
			camera.grace_space = PI-PI/(1.79*2.0*camera.f);
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

	// toggle mouse capture
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && !esc_down){
		esc_down = 1;
		glfwSetInputMode(window, GLFW_CURSOR, glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);//toggle cursor
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	}
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE){
		esc_down = 0;
	}

	//update currChunk
	{
		std::lock_guard<std::mutex> lock(currChunk_mutex);
		currChunk.x = worldToChunk(camera.xyz[0]);
		currChunk.y = worldToChunk(camera.xyz[1]);
		currChunk.z = worldToChunk(camera.xyz[2]);
	}
	updateThreadCV.notify_one();
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
