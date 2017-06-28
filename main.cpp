// OpenGL headers
#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/gl.h>

// SDL headers
#include <SDL2/SDL_main.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>

#include "Spine.h"

bool quit;

SDL_Window* window;
SDL_GLContext glContext;
SDL_Event sdlEvent;

int main(int argc, char *argv[])
{
    
    quit = false;

    //Use OpenGL 3.1 core
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Initialize video subsystem
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        // Display error message
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    else
    {
        // Create window
        window = SDL_CreateWindow("Hello World!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 768, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
        if( window == NULL )
        {
            // Display error message
            printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
            return false;
        }
        else
        {
            // Create OpenGL context
            glContext = SDL_GL_CreateContext(window);

            if( glContext == NULL )
            {
                // Display error message
                printf( "OpenGL context could not be created! SDL Error: %s\n", SDL_GetError() );
                return false;
            }
            else
            {
                // Initialize glew
                glewInit();
            }
        }
    }
    
    // Enable texture and Blending
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
        
    // Load the atlas from a file. The last argument is a void* that will be 
    // stored in atlas->rendererObject.
    spAtlas* atlas = spAtlas_createFromFile("goblins.atlas", 0);

    // load the skeleton from a binary file
    spSkeletonBinary* binary = spSkeletonBinary_create(atlas);

    // Scale for the skeleton
    binary->scale = .0047;

    // Load the skeleton .skel file into a spSkeletonData
    spSkeletonData* skeletonData = spSkeletonBinary_readSkeletonDataFile(binary, "goblins-ess.skel");

    // Create the skeleton
    spSkeleton* skeleton = spSkeleton_create(skeletonData);

    // Set the skeleton skin
    spSkeleton_setSkinByName(skeleton, "goblingirl");
    
    // If loading failed, print the error and exit the app
    if (!skeletonData) {
        exit(0);
    }

    // create an animation state data
    spAnimationStateData *stateData = spAnimationStateData_create(skeletonData);

    // create an animation state
    spAnimationState *animationState = spAnimationState_create(stateData);

    // create a track with the "walk" animation
    spTrackEntry* track =  spAnimationState_addAnimationByName (animationState, 0, "walk", 1, 0.1);    
    
    
    // Game loop
    while (!quit)
    {
        while(SDL_PollEvent(&sdlEvent) != 0)
        {
            // Esc button is pressed
            if(sdlEvent.type == SDL_QUIT)
            {
                quit = true;
            }
        }

        // apply animation
        spAnimationState_apply(animationState, skeleton);
        // update animation state
        spAnimationState_update(animationState, 0.016);
        // update skeleton
        spSkeleton_updateWorldTransform(skeleton);
        
        // Set background color as cornflower blue
        glClearColor(0.39f, 0.58f, 0.93f, 1.f);
        // Clear color buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                

        // change the model matrix to move the model a little
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();        
        glTranslatef(0, -.4, 0);
        
        // draw the skeleton
        drawSkeleton(skeleton);
            
        // Update window with OpenGL rendering
        SDL_GL_SwapWindow(window);
    }

    //Destroy window
    SDL_DestroyWindow(window);
    window = NULL;

    //Quit SDL subsystems
    SDL_Quit();

    return 0;
} 

