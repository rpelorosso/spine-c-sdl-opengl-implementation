#pragma once

#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/gl.h>

// SDL headers
#include <SDL2/SDL_main.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>

// spine headers
#include "spine/spine.h"
#include "spine/extension.h"
#include "spine/Slot.h"

typedef struct Texture {
   // ... OpenGL handle, image data, whatever ...
   GLuint gl_texture;
   int width;
   int height;
} Texture;


void drawSkeleton(spSkeleton* skeleton);
Texture* engine_loadTexture(const char* file);
void engine_disposeTexture(Texture* texture);

