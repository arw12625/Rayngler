#include <stdlib.h>

#include "double_pbo_texture.h"

const GLenum PBO_PIXEL_FORMAT = GL_RGBA;
const int PBO_NUM_CHANNELS = 4;

void initDoublePBO(DoublePBO **dpboRef, int texWidth, int texHeight) {
	
	DoublePBO *dpbo = malloc(sizeof(DoublePBO));
	*dpboRef = dpbo;
	
	dpbo->texWidth = texWidth;
	dpbo->texHeight = texHeight;
	dpbo->dataSize = PBO_NUM_CHANNELS * texWidth * texHeight;
	
	GLubyte *imageData = calloc(dpbo->dataSize, sizeof(GLubyte));
    
	dpbo->frame = 0;
	
	//setup texture
	glGenTextures(1, &dpbo->textureID);
    glBindTexture(GL_TEXTURE_2D, dpbo->textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, dpbo->texWidth, dpbo->texHeight, 0, PBO_PIXEL_FORMAT, GL_UNSIGNED_BYTE, (GLvoid*)imageData);
    #ifdef __EMSCRIPTEN__
    // OpenGL ES 2.0, pixel buffers are not supported well
    // So when using Emscripten, we do not use double buffering
    dpbo->pixels = imageData;
    #else
	//setup pbos
	glGenBuffers(2, dpbo->pboIDs);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, dpbo->pboIDs[0]);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, dpbo->dataSize, 0, GL_STREAM_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, dpbo->pboIDs[1]);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, dpbo->dataSize, 0, GL_STREAM_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	free(imageData);
	#endif
    
}

void updateDoublePBO(DoublePBO *dpbo, void (*writeTex)(GLubyte*, int, GLenum)) {
    #ifdef __EMSCRIPTEN__
	glBindTexture(GL_TEXTURE_2D, dpbo->textureID);
    writeTex(dpbo->pixels, dpbo->dataSize, PBO_PIXEL_FORMAT);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, dpbo->texWidth, dpbo->texHeight, PBO_PIXEL_FORMAT, GL_UNSIGNED_BYTE, (GLvoid*)dpbo->pixels);
    //glBindTexture(GL_TEXTURE_2D, 0);
    
    #else
	int activePBO = dpbo->frame;
	int inactivePBO = 1 - dpbo->frame;
	
	// write texture from active pbo
	glBindTexture(GL_TEXTURE_2D, dpbo->textureID);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, dpbo->pboIDs[activePBO]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, dpbo->texWidth, dpbo->texHeight, PBO_PIXEL_FORMAT, GL_UNSIGNED_BYTE, 0);
	
	// write inactive pbo from callback
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, dpbo->pboIDs[inactivePBO]);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, dpbo->dataSize, 0, GL_STREAM_DRAW);
    GLubyte* ptr = (GLubyte*)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, dpbo->dataSize, GL_MAP_WRITE_BIT );
	if(ptr) {
        // update data directly on the mapped buffer
        writeTex(ptr, dpbo->dataSize, PBO_PIXEL_FORMAT);
        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);  // release pointer to mapping buffer
    }
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	
	dpbo->frame = inactivePBO;
    #endif
	
}

void destroyDoublePBO(DoublePBO *dpbo) {
	
    #ifdef EMEMEM
    free(dpbo->pixels);
    #endif
	//maybe should release pbo and texture
	
	free(dpbo);
}