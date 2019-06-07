#version 330 core

in vec2 vert;
in vec2 texCoordIn;

out vec2 texCoord;

uniform mat4 pm;

void main()
{
    texCoord = texCoordIn;
    gl_Position = pm * vec4(vert, 0.0, 1.0);
}