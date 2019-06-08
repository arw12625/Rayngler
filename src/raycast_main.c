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
#include "double_pbo_texture.h"
#include "graphics_2d.h"

void init();
void render();
void terminate();
void update(double delta);
void updatePixels(GLubyte* dst, int size, GLenum format);
void glInit();
char* filetobuf(const char *file);

const char vertShaderSource[] = "res/simple2D.vert";
const char fragShaderSource[] = "res/simple2D.frag";

const int    SCREEN_WIDTH    = 1024;
const int    SCREEN_HEIGHT   = 768;
const int    IMAGE_WIDTH     = 1024;
const int    IMAGE_HEIGHT    = 1024;

GLFWwindow* window;


RayCastWorld *world;
RayCastSettings settings;

DoublePBO *dpbo;

Graphics2D *g2D;

int main(void) {

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Hello World", NULL, NULL);
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
	world->player->stepHeight = 0.35;
	world->gravity = -0.001;
	settings.screenWidth = IMAGE_HEIGHT;
	settings.screenHeight = IMAGE_WIDTH;
	settings.cosXFOV = 0.5;
	settings.cosYFOV = 0.5;
	settings.castLimit = 25;
	
    
	//printf("GLINIT\n");
	glInit();
	GLchar *vertexsource, *fragmentsource;
	vertexsource = filetobuf(vertShaderSource);
    fragmentsource = filetobuf(fragShaderSource);
	initGraphics2D(&g2D, SCREEN_WIDTH, SCREEN_HEIGHT, 0, vertexsource, fragmentsource);
	
	initDoublePBO(&dpbo, IMAGE_HEIGHT, IMAGE_WIDTH);
	
	g2D->textureID = dpbo->textureID;
	
}

void glInit() {
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
	
	
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	
}


void render() {
	/* Render here */
    glClear(GL_COLOR_BUFFER_BIT);
	
	updateDoublePBO(dpbo, GL_TEXTURE0, &updatePixels); 
	
	renderGraphics2D(g2D);

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
	
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}

void terminate(int delta) {
	destroyDoublePBO(dpbo);
	destroyGraphics2D(g2D);
	destroyWorld(world);
}

void updatePixels(GLubyte* dst, int size, GLenum format) {
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
