#ifdef __EMSCRIPTEN__
// For emscripten, instead of using glad we use its built-in support for OpenGL:
#include "emscripten.h"
#include <GLES3/gl3.h>
#else
#include "glad/glad.h"
#endif