The pixels of each frame are computed on the CPU, then written to the GPU using a double buffer of pixel buffer objects to be displayed as texture by opengl.


# Rayngler
A simple raycasting engine written in C using opengl.
It currently supports multiple wall and player heights, wall transparency, and looking up and down.
The project can be built as an executable or as a webpage.

## Building
The project is built using cmake.

### Build an executable
The executable depends on [glad](https://glad.dav1d.de/) and [GLFW3](https://www.glfw.org/).
These dependencies should be put in a library directory one level up from the project directory (or edit the cmake file).
You can then build the project as an executable with the following commands:

    mkdir build
    cd build
    cmake ..
    make

You can specify different makefile generators with the `-G` option in cmake, for example `cmake .. -G "MinGW Makefiles"`.
Then you can open the executable in the `bin` folder.

### Build a web page
You can build the project as a web page using cmake and emscripten.
The easiest way to do this is with Docker.
On Windows this can be done with:

    docker run --rm -v ${pwd}:/src emscripten/emsdk ./make_page.bash

You can then view the raycaster on the page `bin/index.html`.
Becuase of restrictions on cross-origin resource sharing, this page must be accessed from a server.
You can create a simple local server with Python 3 with the command:

    python -m http.server
    
The page can then be accessed with a web browser by navigating to `http://localhost:8000/path/to/bin/index.html`

Note when switching build targets, you will need to delete `CmakeCache.txt`
