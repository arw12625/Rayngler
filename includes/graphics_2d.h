#ifndef GRAPHICS_2D_H
#define GRAPHICS_2D_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

typedef struct Graphics2D {
	int screenWidth, screenHeight;
	
	GLuint vertexshader, fragmentshader;
	GLuint shaderprogram;
	
	GLuint vertBufferID;
	GLuint texCoordBufferID;
	GLuint elementID;
	GLuint vaoID;
	
	int numElements;
	
	GLuint vertAttr;
	GLuint texCoordAttr;
	GLuint texUnit;
	GLuint pmLoc;
	
	GLuint textureID;
	
} Graphics2D;


void initGraphics2D(Graphics2D ** g2DRef, int screenWidth, int screenHeight, int rotated, GLchar *vertexsource, GLchar *fragmentsource);

void renderGraphics2D(Graphics2D *g2D);

void destroyGraphics2D(Graphics2D *g2D);

#endif