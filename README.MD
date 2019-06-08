Rayngler

A simple raycasting engine written in C using GLFW and GLAD. Support for multiple wall and player heights, wall transparency, and looking up and down.

The pixels of each frame are computed on the CPU, then written to the GPU using a double buffer of pixel buffer objects to be displayed as texture by opengl.