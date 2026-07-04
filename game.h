#pragma once
#include "raylib.h"

#define BLOCK_SIZE       2.0f
#define NUM_CHUNKS       7
#define CHUNK_WIDTH      16*NUM_CHUNKS
#define CHUNK_HEIGHT     20
#define MAX_BUILD_HEIGHT 40
#define CHUNK_AREA       (CHUNK_WIDTH * CHUNK_WIDTH) 
#define CHUNK_SIZE       (CHUNK_AREA * CHUNK_HEIGHT)
#define MAX_CHUNK_SIZE   (CHUNK_AREA * MAX_BUILD_HEIGHT)

typedef struct {
    Vector3 loc;
    Vector3 loc_cube;
    Rectangle *texture;
    BoundingBox collision_box;
    bool not_air;
} Block;

typedef struct {
    Block cubes[MAX_CHUNK_SIZE];
} Chunk;

typedef enum {
    FACE_FRONT, FACE_BACK,
    FACE_TOP, FACE_BOTTOM,
    FACE_RIGHT, FACE_LEFT,
    FACE_NONE
} Faces;
