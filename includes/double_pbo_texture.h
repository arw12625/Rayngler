#ifndef DOUBLE_PBO_TEXTURE_H
#define DOUBLE_PBO_TEXTURE_H

#include "opengl.h"
#include <GLFW/glfw3.h>

typedef struct DoublePBO {
	
	int texWidth, texHeight;
	int dataSize;
	
    GLuint pboIDs[2];
	GLuint textureID;
	
	int frame;
    
    #ifdef __EMSCRIPTEN__
    GLubyte *pixels;
    #endif
	
} DoublePBO;

void initDoublePBO(DoublePBO **dpboRef, int texWidth, int texHeight);

void updateDoublePBO(DoublePBO *dpbo, void (*writeTex)(GLubyte*, int, GLenum));

void destroyDoublePBO(DoublePBO *dpbo);

#endif