cmake_minimum_required(VERSION 3.3)

project(Raycaster)

if(EMSCRIPTEN)
set(EXECUTABLE_OUTPUT_PATH "bin/js")
set(HTML_OUTPUT_PATH "bin")
else()
set(EXECUTABLE_OUTPUT_PATH "bin")
endif()

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/includes")
set(RES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/res")
set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../libraries")

set(SCREEN_WIDTH "1024" CACHE STRING "Width of the screen/window to create.")
set(SCREEN_HEIGHT "768" CACHE STRING "Height of the screen/window to create.")
set(CMAKE_C_FLAGS "-DSCREEN_WIDTH=${SCREEN_WIDTH} -DSCREEN_HEIGHT=${SCREEN_HEIGHT}")

add_executable(Raycaster src/raycast_main.c ${SRC_DIR}/raycast.c ${INC_DIR}/raycast.h ${SRC_DIR}/mapGen.c ${INC_DIR}/mapGen.h ${SRC_DIR}/vecMath.c ${INC_DIR}/vecMath.h ${SRC_DIR}/double_pbo_texture.c ${INC_DIR}/double_pbo_texture.h ${SRC_DIR}/graphics_2d.c ${INC_DIR}/graphics_2d.h)

target_include_directories(Raycaster PRIVATE ${INC_DIR})

if(EMSCRIPTEN)
    target_link_options(Raycaster PRIVATE -sUSE_GLFW=3 -s WASM=1 -sMIN_WEBGL_VERSION=2)
    add_custom_command(
        TARGET Raycaster
        COMMAND ${CMAKE_COMMAND} -E copy
        ${SRC_DIR}/index.html
        ${HTML_OUTPUT_PATH}/index.html
        DEPENDS ${SRC_DIR}/index.html
    )
    
    # Build html file from emscripten
    #set(CMAKE_EXECUTABLE_SUFFIX ".html") 
else()
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    add_subdirectory(${LIB_DIR}/glfw ${LIB_DIR}/glfw)
    target_link_libraries(Raycaster PRIVATE glfw)

    add_subdirectory(${LIB_DIR}/glad ${LIB_DIR}/glad)
    target_link_libraries(Raycaster PRIVATE glad)
endif ()