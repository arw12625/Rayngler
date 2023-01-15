The pixels of each frame are computed on the CPU, then written to the GPU using a double buffer of pixel buffer objects to be displayed as texture by opengl.


# Rayngler
A simple raycasting engine written in C using opengl.
It currently supports multiple wall and player heights, wall transparency, and looking up and down.
The project can be built as an executable or as a webpage.

## Building
The project is built using cmake.

### Build an executable
You can build the project as an executable with the following commands

    mkdir build
    cd build
    cmake ..
    make

You can specify different makefile generators with the `-G` option in cmake, for example `cmake .. -G "MinGW Makefiles"`.

### Build a web page
You can build the project as a web page using cmake and emscripten.
The easiest way to do this is with Docker.
On Windows this can be done with

    docker run --rm -v ${pwd}:/src emscripten/emsdk ./make_page.bash

Or on Linux (untested)

    docker run --rm -v $(pwd):$(pwd) emscripten/emsdk ./make_page.bash

