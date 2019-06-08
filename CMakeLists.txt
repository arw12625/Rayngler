cmake_minimum_required(VERSION 3.3)

project(Raycaster)

set(EXECUTABLE_OUTPUT_PATH "bin")

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/includes")
set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../libraries")

add_executable(Raycaster src/raycast_main.c ${SRC_DIR}/raycast.c ${INC_DIR}/raycast.h ${SRC_DIR}/mapGen.c ${INC_DIR}/mapGen.h ${SRC_DIR}/vecMath.c ${INC_DIR}/vecMath.h)

target_include_directories(Raycaster PRIVATE ${INC_DIR})


set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(${LIB_DIR}/glfw ${LIB_DIR}/glfw)
target_link_libraries(Raycaster PRIVATE glfw)

add_subdirectory(${LIB_DIR}/glad ${LIB_DIR}/glad)
target_link_libraries(Raycaster PRIVATE glad)

