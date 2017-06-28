#include "Spine.h"

void drawSkeleton(spSkeleton* skeleton);


char* _spUtil_readFile (const char* path, int* length){
   return _readFile(path, length);
}


void _spAtlasPage_createTexture (spAtlasPage* self, const char* path){
    Texture* texture = new Texture;
    
    GLuint TextureID = 0;
    
    SDL_Surface* Surface = IMG_Load(path);

    glGenTextures(1, &TextureID);
    glBindTexture(GL_TEXTURE_2D, TextureID);
    
    int Mode = GL_RGB;
    
    if(Surface->format->BytesPerPixel == 4) {
        Mode = GL_RGBA;
    }
    
    glTexImage2D(GL_TEXTURE_2D, 0, Mode, Surface->w, Surface->h, 0, Mode, GL_UNSIGNED_BYTE, Surface->pixels);    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    texture->gl_texture = TextureID;
    texture->width = Surface->w;
    texture->height = Surface->h;

   // store the Texture on the rendererObject so we can
   // retrieve it later for rendering.
   self->rendererObject = texture;

   // store the texture width and height on the spAtlasPage
   // so spine-c can calculate texture coordinates for
   // rendering.
   self->width = texture->width;
   self->height = texture->height;
}

void _spAtlasPage_disposeTexture (spAtlasPage* self) {
   // if the rendererObject is not set, loading failed
   // so we do not need to dispose of anything.
   if (!self->rendererObject) return;

   // Dispose the texture
   Texture* texture = (Texture*)self->rendererObject;
  // engine_disposeTexture(texture);
}

// A single vertex with UV 
typedef struct Vertex {
   // Position in x/y plane
   float x, y;

   // UV coordinates
   float u, v;

   // Color, each channel in the range from 0-1
   // (Should really be a 32-bit RGBA packed color)
   float r, g, b, a;
} Vertex;

enum BlendMode {
   // See https://github.com/EsotericSoftware/spine-runtimes/blob/master/spine-libgdx/spine-libgdx/src/com/esotericsoftware/spine/BlendMode.java#L37
   // for how these translate to OpenGL source/destination blend modes.
   BLEND_NORMAL,
   BLEND_ADDITIVE,
   BLEND_MULTIPLY,      
   BLEND_SCREEN,   
};

// Draw the given mesh.
// - vertices is a pointer to an array of Vertex structures
// - start defines from which vertex in the vertices array to start
// - count defines how many vertices to use for rendering (should be divisible by 3, as we render triangles, each triangle requiring 3 vertices)
// - texture the texture to use
// - blendMode the blend mode to use
void engine_drawMesh(Vertex* vertices, int start, int count, Texture* texture, BlendMode blendmode);




#define MAX_VERTICES_PER_ATTACHMENT 2048
float worldVerticesPositions[MAX_VERTICES_PER_ATTACHMENT];
Vertex vertices[MAX_VERTICES_PER_ATTACHMENT];

// Little helper function to add a vertex to the scratch buffer. Index will be increased
// by one after a call to this function.
void addVertex(float x, float y, float u, float v, float r, float g, float b, float a, int* index) {
   Vertex* vertex = &vertices[*index];
   vertex->x = x;
   vertex->y = y;
   vertex->u = u;
   vertex->v = v;
   vertex->r = r;
   vertex->g = g;
   vertex->b = b;
   vertex->a = a;
   *index += 1;
}

#define SP_VERTEX_X1 0
#define SP_VERTEX_Y1 1 


void drawSkeleton(spSkeleton* skeleton) {
   // For each slot in the draw order array of the skeleton
   for (int i = 0; i < skeleton->slotsCount; ++i) {
      spSlot* slot = skeleton->drawOrder[i];

      // Fetch the currently active attachment, continue
      // with the next slot in the draw order if no
      // attachment is active on the slot
      spAttachment* attachment = slot->attachment;
      if (!attachment) continue;

      // Fetch the blend mode from the slot and
      // translate it to the engine blend mode
      BlendMode engineBlendMode;
      switch (slot->data->blendMode) {
         case SP_BLEND_MODE_NORMAL:
            engineBlendMode = BLEND_NORMAL;
            break;
         case SP_BLEND_MODE_ADDITIVE:
            engineBlendMode = BLEND_ADDITIVE;
            break;
         case SP_BLEND_MODE_MULTIPLY:
            engineBlendMode = BLEND_MULTIPLY;
            break;
         case SP_BLEND_MODE_SCREEN:
            engineBlendMode = BLEND_SCREEN;
            break;
         default:
            // unknown Spine blend mode, fall back to
            // normal blend mode
            engineBlendMode = BLEND_NORMAL;
      }

      // Calculate the tinting color based on the skeleton's color
      // and the slot's color. Each color channel is given in the
      // range [0-1], you may have to multiply by 255 and cast to
      // and int if your engine uses integer ranges for color channels.
      float tintR = skeleton->color.r * slot->color.r;
      float tintG = skeleton->color.g * slot->color.g;
      float tintB = skeleton->color.b * slot->color.b;
      float tintA = skeleton->color.a * slot->color.a;

      // Fill the vertices array depending on the type of attachment
      Texture* texture = 0;
      int vertexIndex = 0;
      if (attachment->type == SP_ATTACHMENT_REGION) {
         // Cast to an spRegionAttachment so we can get the rendererObject
         // and compute the world vertices
         spRegionAttachment* regionAttachment = (spRegionAttachment*)attachment;

         // Our engine specific Texture is stored in the spAtlasRegion which was
         // assigned to the attachment on load. It represents the texture atlas
         // page that contains the image the region attachment is mapped to
         texture = (Texture*)((spAtlasRegion*)regionAttachment->rendererObject)->page->rendererObject;

         // Computed the world vertices positions for the 4 vertices that make up
         // the rectangular region attachment. This assumes the world transform of the
         // bone to which the slot (and hence attachment) is attached has been calculated
         // before rendering via spSkeleton_updateWorldTransform
         spRegionAttachment_computeWorldVertices(regionAttachment, slot->bone, worldVerticesPositions, 0, 2);

         // Create 2 triangles, with 3 vertices each from the region's
         // world vertex positions and its UV coordinates (in the range [0-1]).
         addVertex(worldVerticesPositions[SP_VERTEX_X1], worldVerticesPositions[SP_VERTEX_Y1],
                regionAttachment->uvs[SP_VERTEX_X1], regionAttachment->uvs[SP_VERTEX_Y1],
                tintR, tintG, tintB, tintA, &vertexIndex);

         addVertex(worldVerticesPositions[0], worldVerticesPositions[1],
                regionAttachment->uvs[0], regionAttachment->uvs[1],
                tintR, tintG, tintB, tintA, &vertexIndex);

         addVertex(worldVerticesPositions[2], worldVerticesPositions[3],
                regionAttachment->uvs[2], regionAttachment->uvs[3],
                tintR, tintG, tintB, tintA, &vertexIndex);

         addVertex(worldVerticesPositions[4], worldVerticesPositions[5],
                regionAttachment->uvs[4], regionAttachment->uvs[5],
                tintR, tintG, tintB, tintA, &vertexIndex);

         addVertex(worldVerticesPositions[6], worldVerticesPositions[7],
                regionAttachment->uvs[6], regionAttachment->uvs[7],
                tintR, tintG, tintB, tintA, &vertexIndex);

         addVertex(worldVerticesPositions[0], worldVerticesPositions[1],
                regionAttachment->uvs[0], regionAttachment->uvs[1],
                tintR, tintG, tintB, tintA, &vertexIndex);
      } else if (attachment->type == SP_ATTACHMENT_MESH) {
         // Cast to an spMeshAttachment so we can get the rendererObject
         // and compute the world vertices
         spMeshAttachment* mesh = (spMeshAttachment*)attachment;

         // Check the number of vertices in the mesh attachment. If it is bigger
         // than our scratch buffer, we don't render the mesh. We do this here
         // for simplicity, in production you want to reallocate the scratch buffer
         // to fit the mesh.
         if (mesh->super.worldVerticesLength > MAX_VERTICES_PER_ATTACHMENT) continue;

         // Our engine specific Texture is stored in the spAtlasRegion which was
         // assigned to the attachment on load. It represents the texture atlas
         // page that contains the image the mesh attachment is mapped to
         texture = (Texture*)((spAtlasRegion*)mesh->rendererObject)->page->rendererObject;

         // Computed the world vertices positions for the vertices that make up
         // the mesh attachment. This assumes the world transform of the
         // bone to which the slot (and hence attachment) is attached has been calculated
         // before rendering via spSkeleton_updateWorldTransform
         spVertexAttachment_computeWorldVertices(SUPER(mesh), slot, 0, mesh->super.worldVerticesLength, worldVerticesPositions, 0, 2);

         // Mesh attachments use an array of vertices, and an array of indices to define which
         // 3 vertices make up each triangle. We loop through all triangle indices
         // and simply emit a vertex for each triangle's vertex.
         for (int i = 0; i < mesh->trianglesCount; ++i) {
            int index = mesh->triangles[i] << 1;
            addVertex(worldVerticesPositions[index], worldVerticesPositions[index + 1],
                   mesh->uvs[index], mesh->uvs[index + 1], 
                   tintR, tintG, tintB, tintA, &vertexIndex);
         }

      }

      // Draw the mesh we created for the attachment
      engine_drawMesh(vertices, 0, vertexIndex, texture, engineBlendMode);
   }
}

void engine_drawMesh(Vertex* vertices, int start, int count, Texture* texture, BlendMode blendmode)
{
    int i;

    // bind texture
    glBindTexture(GL_TEXTURE_2D, texture->gl_texture);

    // set the blending function
    // TODO: change according to BlendMode
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    
    // render the polygons
    glBegin(GL_TRIANGLE_STRIP);
    for (i = 0; i < count; i++) {
        glColor4f(vertices[i].r,vertices[i].b,vertices[i].b,vertices[i].a);
        glTexCoord2f(vertices[i].u, vertices[i].v);
        glVertex3f(vertices[i].x, vertices[i].y, 0);
    }
    glEnd();
}


