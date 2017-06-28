cmake_minimum_required(VERSION 2.6)
project(spine)

link_directories(/home/silencer/Desktop/squirrel)
include_directories(/home/silencer/Desktop/squirrel/includes ${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB SPINE_SRC "spine/*.c") # add spine sources

add_executable(spine ${SPINE_SRC} Spine.cpp main.cpp)

install(TARGETS spine RUNTIME DESTINATION bin)
TARGET_LINK_LIBRARIES(spine SDL2 OpenGL GLEW squirrel sqstdlib SDL2_image SDL2_ttf)
 
