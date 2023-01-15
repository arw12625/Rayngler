#include <stdlib.h>
#include <stdio.h>

#include "graphics_2d.h"

static const GLfloat vertData[] = {
    -1.0f,  1.0f,
    -1.0f, -1.0f,
     1.0f, -1.0f,
     1.0f,  1.0f
};

static const GLuint elementData[] = {
    0,1,2,2,3,0
};
const GLfloat gpm[] = {
    1.0f , 0.0f , 0.0f , 0.0f,
    0.0f, 1.0f, 0.0f , 0.0f,
    0.0f, 0.0f, 0, 0,
    0.0f, 0.0f, 0.0f, 1.0f
};

static const GLfloat texCoordData[] = { 
    0,0,
	0,1,
	1,1,
	1,0
};

static const GLfloat rotTexCoordData[] = { 
    0,0,
	1,0,
	1,1,
	0,1
};

void loadShaderProgramG2D(Graphics2D *g2D, const GLchar *vertexsource, const GLchar *fragmentsource);

void initGraphics2D(Graphics2D ** g2DRef, int screenWidth, int screenHeight, int rotated, const GLchar *vertexsource, const GLchar *fragmentsource) {

	Graphics2D *g2D = malloc(sizeof(Graphics2D));
	*g2DRef = g2D;
	
	g2D->screenWidth = screenWidth;
	g2D->screenHeight = screenHeight;
	
	g2D->numElements = 6;
	g2D->vertAttr = 0;
	g2D->texCoordAttr = 1;
    g2D->texUnit = 0;
	
	glGenVertexArrays(1, &g2D->vaoID);
    glBindVertexArray(g2D->vaoID);
    
	glGenBuffers(1,&g2D->vertBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, g2D->vertBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertData), vertData, GL_STATIC_DRAW);
	glVertexAttribPointer(g2D->vertAttr, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(g2D->vertAttr);
	
	glGenBuffers(1,&g2D->texCoordBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, g2D->texCoordBufferID);
	if(rotated) {
		glBufferData(GL_ARRAY_BUFFER, sizeof(texCoordData), texCoordData, GL_STATIC_DRAW);
	} else {
		glBufferData(GL_ARRAY_BUFFER, sizeof(rotTexCoordData), rotTexCoordData, GL_STATIC_DRAW);
	}
	glVertexAttribPointer(g2D->texCoordAttr, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(g2D->texCoordAttr);
	
    glGenBuffers(1, &g2D->elementID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g2D->elementID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elementData), elementData, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	loadShaderProgramG2D(g2D, vertexsource, fragmentsource);
	
    
}

void activateTextureUnit(Graphics2D *g2D) {
    glActiveTexture(GL_TEXTURE0 + g2D->texUnit);
}


void renderGraphics2D(Graphics2D *g2D) {
    glUseProgram(g2D->shaderprogram);
    activateTextureUnit(g2D);
    glBindTexture(GL_TEXTURE_2D, g2D->textureID);
    
	glDrawElements(GL_TRIANGLES, g2D->numElements, GL_UNSIGNED_INT, 0);
}

void destroyGraphics2D(Graphics2D *g2D) {
	
	// maybe should free gl buffers/objects
    glDeleteVertexArrays(1, &g2D->vaoID);
	
	free(g2D);
}

void loadShaderProgramG2D(Graphics2D *g2D, const GLchar *vertexsource, const GLchar *fragmentsource) {
	
	g2D->vertexshader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(g2D->vertexshader, 1, (const GLchar**)&vertexsource, 0);
	glCompileShader(g2D->vertexshader);
	GLint success = 0;
	glGetShaderiv(g2D->vertexshader, GL_COMPILE_STATUS, &success);
	if(success == GL_FALSE) {
		printf("Vertex Shader did not compile correctly\n");
	}
	
	g2D->fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(g2D->fragmentshader, 1, (const GLchar**)&fragmentsource, 0);
	glCompileShader(g2D->fragmentshader);
	glGetShaderiv(g2D->fragmentshader, GL_COMPILE_STATUS, &success);
	if(success == GL_FALSE) {
		printf("Fragment Shader did not compile correctly\n");
	}
	
	g2D->shaderprogram = glCreateProgram();
	glAttachShader(g2D->shaderprogram, g2D->vertexshader);
    glAttachShader(g2D->shaderprogram, g2D->fragmentshader);
	
	glBindAttribLocation(g2D->shaderprogram, g2D->vertAttr, "vert");
    glBindAttribLocation(g2D->shaderprogram, g2D->texCoordAttr, "texCoordIn");
	
	glLinkProgram(g2D->shaderprogram);
	
	glGetProgramiv(g2D->shaderprogram, GL_LINK_STATUS, &success);
	if(success == GL_FALSE) {
		printf("Shader did not link correctly\n");
	}
	
    glUseProgram(g2D->shaderprogram);
	
    g2D->pmLoc = glGetUniformLocation(g2D->shaderprogram, "pm");
    glUniformMatrix4fv(g2D->pmLoc, 1, GL_FALSE, gpm);
    
	glUniform1i(glGetUniformLocation(g2D->shaderprogram, "tex"), g2D->texUnit);
    
}