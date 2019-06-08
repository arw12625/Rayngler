#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "raycast.h"
#include "mapGen.h"
#include "vecMath.h"

void init();
void render();
void terminate();
void update(double delta);
void updatePixels(GLubyte* dst, int size);
void glInit();
void loadShaderProgram();
char* filetobuf(const char *file);

const char vertShaderSource[] = "res/simple2D.vert";
const char fragShaderSource[] = "res/simple2D.frag";

const int    SCREEN_WIDTH    = 1024;
const int    SCREEN_HEIGHT   = 1024;
const int    IMAGE_WIDTH     = 1024;
const int    IMAGE_HEIGHT    = 1024;
const int    CHANNEL_COUNT   = 4;
const int    DATA_SIZE       = IMAGE_WIDTH * IMAGE_HEIGHT * CHANNEL_COUNT;
const GLenum PIXEL_FORMAT    = GL_RGBA;

GLFWwindow* window;

GLuint vertexshader, fragmentshader;
GLuint shaderprogram;

GLuint pboIDs[2];
GLuint textureID;
GLubyte *imageData = 0;
GLuint vertBufferID;
GLuint texCoordBufferID;
GLuint elementID;
GLuint vaoID;

const GLuint vertAttr = 0;
const GLuint texCoordAttr = 1;
const GLuint texNum = 0;
GLuint pmLoc = 0;


RayCastWorld *world;
RayCastSettings settings;


static const GLfloat vertData[] = {
    -1.0f,  1.0f,
    -1.0f, -1.0f,
     1.0f, -1.0f,
     1.0f,  1.0f
};
static const GLfloat texCoordData[] = { 
    0,0,
	1,0,
	1,1,
	0,1
};
static const GLuint elementData[] = {
    0,1,2,3
};
const GLfloat pm[] = {
    1.0f , 0.0f , 0.0f , 0.0f,
    0.0f, 1.0f, 0.0f , 0.0f,
    0.0f, 0.0f, 0, 0,
    0.0f, 0.0f, 0.0f, 1.0f
};

int main(void) {

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

	int version = gladLoadGL();
	printf("GL Version: %d", version);

	//printf("INIT\n");

	init();
	
	clock_t currentClock, oldClock;
	double delta = 0;
	oldClock = clock();

	//printf("LOOP\n");
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
		currentClock = clock();
		delta = (double)(currentClock - oldClock) / CLOCKS_PER_SEC * 1000.0;
		oldClock = currentClock;
		
		update(delta);
		
		//printf("Render\n");
        render();
		//printf("POST Render\n");
    }
	
	terminate();

    glfwTerminate();
    return 0;
}

void init() {
	//generateGratedWorld(&world, 20,20);
	generateGratedWorld(&world, 10,10);
	world->player->position.x = 5.5;
	world->player->position.y = 7.5;
	world->player->height = 1.5;
	world->gravity = -0.001;
	settings.screenWidth = IMAGE_HEIGHT;
	settings.screenHeight = IMAGE_WIDTH;
	settings.cosXFOV = 0.5;
	settings.cosYFOV = 0.5;
	settings.castLimit = 25;
	
    imageData = calloc(DATA_SIZE, sizeof(GLubyte));	
	
	//printf("GLINIT\n");
	glInit();
	
	free(imageData);
}

void glInit() {
	
	glActiveTexture(GL_TEXTURE0);
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
	
	loadShaderProgram();
	
	glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	//might need to swap height and width
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, IMAGE_HEIGHT, IMAGE_WIDTH, 0, PIXEL_FORMAT, GL_UNSIGNED_BYTE, (GLvoid*)imageData);
    glBindTexture(GL_TEXTURE_2D, 0);
	
	glGenBuffers(2, pboIDs);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboIDs[0]);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, DATA_SIZE, 0, GL_STREAM_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboIDs[1]);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, DATA_SIZE, 0, GL_STREAM_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	
	
	glGenVertexArrays(1, &vaoID);
	glBindVertexArray(vaoID);
	
	glGenBuffers(1,&vertBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, vertBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertData), vertData, GL_STATIC_DRAW);
	glVertexAttribPointer(vertAttr, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vertAttr);
	
	glGenBuffers(1,&texCoordBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, texCoordBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texCoordData), texCoordData, GL_STATIC_DRAW);
	glVertexAttribPointer(texCoordAttr, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(texCoordAttr);
	
	glGenBuffers(1, &elementID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elementData), elementData, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	
}

void loadShaderProgram() {
	
	GLchar *vertexsource, *fragmentsource;

	vertexsource = filetobuf(vertShaderSource);
    fragmentsource = filetobuf(fragShaderSource);
	
	vertexshader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexshader, 1, (const GLchar**)&vertexsource, 0);
	glCompileShader(vertexshader);
	GLint success = 0;
	glGetShaderiv(vertexshader, GL_COMPILE_STATUS, &success);
	if(success == GL_FALSE) {
		printf("Vertex Shader did not compile correctly\n");
	}
	
	fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentshader, 1, (const GLchar**)&fragmentsource, 0);
	glCompileShader(fragmentshader);
	glGetShaderiv(fragmentshader, GL_COMPILE_STATUS, &success);
	if(success == GL_FALSE) {
		printf("Fragment Shader did not compile correctly\n");
	}
	
    free(vertexsource);
    free(fragmentsource);
	
	shaderprogram = glCreateProgram();
	glAttachShader(shaderprogram, vertexshader);
    glAttachShader(shaderprogram, fragmentshader);
	
	glBindAttribLocation(shaderprogram, vertAttr, "vert");
    glBindAttribLocation(shaderprogram, texCoordAttr, "texCoordIn");
	
	glLinkProgram(shaderprogram);
	
	glGetProgramiv(shaderprogram, GL_LINK_STATUS, &success);
	if(success == GL_FALSE) {
		printf("Shader did not link correctly\n");
	}
	
	glUseProgram(shaderprogram);
	
	pmLoc = glGetUniformLocation(shaderprogram, "pm");
	glUniformMatrix4fv(pmLoc, 1, GL_FALSE, pm);
	
	glUniform1i(glGetUniformLocation(shaderprogram, "tex"), texNum);
}

void render() {
	/* Render here */
    glClear(GL_COLOR_BUFFER_BIT);
	
	static int pboFrame = 0;
	pboFrame = 1 - pboFrame;
	int pboNextFrame = 1 - pboFrame;
	

	glBindTexture(GL_TEXTURE_2D, textureID);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboIDs[pboFrame]);	
	//might need to swap image height and width
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, IMAGE_HEIGHT, IMAGE_WIDTH, PIXEL_FORMAT, GL_UNSIGNED_BYTE, 0);
	
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboIDs[pboNextFrame]);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, DATA_SIZE, 0, GL_STREAM_DRAW);
    GLubyte* ptr = (GLubyte*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
	if(ptr) {
        // update data directly on the mapped buffer
        updatePixels(ptr, DATA_SIZE);
        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);  // release pointer to mapping buffer
    }
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	
	glUseProgram(shaderprogram);
    glBindTexture(GL_TEXTURE_2D, textureID);
	//glBindVertexArray(vaoID);
	
	glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, 0);
	

    /* Swap front and back buffers */
    glfwSwapBuffers(window);

}

void update(double delta) {
	
	printf("Delta: %f\n", delta);
	//printf("Height: %f\n", world->player->position.z);
	
    /* Poll for and process events */
	glfwPollEvents();
	
	float camRot = 0;
	float camRotSpeed = 0.03;
	if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		camRot++;
	}
	if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
		camRot--;
	}
	float newCamX = cos(camRot * camRotSpeed) * world->player->camDir.x + -sin(camRot * camRotSpeed) * world->player->camDir.y;
	float newCamY = sin(camRot * camRotSpeed) * world->player->camDir.x + cos(camRot * camRotSpeed) * world->player->camDir.y;
	world->player->camDir.x = newCamX;
	world->player->camDir.y = newCamY;
	
	//printf("camX: %f, camY: %f\n", world->player->camDir.x, world->player->camDir.y);
	//printf("X: %f, Y: %f\n", world->player->x, world->player->y);
	
	
	float moveForward = 0;
	float moveSide = 0;
	float moveSpeed = 0.003;
	if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		moveForward++;
	}
	if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		moveForward--;
	}
	if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		moveSide++;
	}
	if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		moveSide--;
	}
	if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		moveSpeed *= 3;
	}
	
	float moveMag = sqrt(moveForward * moveForward + moveSide * moveSide);
	if(moveMag > 0) {
		moveForward /= moveMag;
		moveSide /= moveMag;
	}
	
	float rotX = world->player->camDir.x;
	float rotY = world->player->camDir.y;
	
	world->player->velocity.x = moveSpeed * (rotX * moveForward - rotY * moveSide);
	world->player->velocity.y = moveSpeed * (rotY * moveForward + rotX * moveSide);
	
	updateWorld(delta, world);
}

void terminate(int delta) {
    glDeleteVertexArrays(1, &vaoID);
	destroyWorld(world);
}

void updatePixels(GLubyte* dst, int size) {
    //static int color = 125215;

    if(!dst) {
        return;
	}

    int* ptr = (int*)dst;

	renderWorld(ptr, world, &settings);

    /*
	// copy 4 bytes at once
    for(int i = 0; i < IMAGE_HEIGHT; ++i)
    {
        for(int j = 0; j < IMAGE_WIDTH; ++j)
        {
            *ptr = color+j;
            ++ptr;
        }
        color += 257;
    }
    ++color;            // scroll down
	*/
}

/* A simple function that will read a file into an allocated char pointer buffer */
char* filetobuf(const char *file) {
    FILE *fptr;
    long length;
    char *buf;

    fptr = fopen(file, "rb"); /* Open file for reading */
    if (!fptr) /* Return NULL on failure */
        return NULL;
    fseek(fptr, 0, SEEK_END); /* Seek to the end of the file */
    length = ftell(fptr); /* Find out how many bytes into the file we are */
    buf = (char*)malloc(length+1); /* Allocate a buffer for the entire length of the file and a null terminator */
    fseek(fptr, 0, SEEK_SET); /* Go back to the beginning of the file */
    fread(buf, length, 1, fptr); /* Read the contents of the file in to the buffer */
    fclose(fptr); /* Close the file */
    buf[length] = 0; /* Null terminator */

    return buf; /* Return the buffer */
}
