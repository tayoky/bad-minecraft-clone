#include "raylib.h"
#include "rlgl.h"
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

#define BLOCK_SIZE       2.0f
#define NUM_CHUNKS       7
#define CHUNK_WIDTH      16*NUM_CHUNKS
#define CHUNK_HEIGHT     20
#define MAX_BUILD_HEIGHT 40
#define CHUNK_AREA       (CHUNK_WIDTH * CHUNK_WIDTH) 
#define CHUNK_SIZE       (CHUNK_AREA * CHUNK_HEIGHT)
#define MAX_CHUNK_SIZE   (CHUNK_AREA * MAX_BUILD_HEIGHT)

typedef enum {
    FACE_FRONT, FACE_BACK,
    FACE_TOP, FACE_BOTTOM,
    FACE_RIGHT, FACE_LEFT
} Faces;

void DrawCubeTextureRec(Texture2D texture, Rectangle *source, Vector3 position, float width, float height, float length, Color color);

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

Rectangle get_texture_rect(Texture2D texture, int x, int y) {
    int cube_sz = texture.width/16;
    return (Rectangle){ cube_sz * x,
                        cube_sz * y,
                        cube_sz,
                        cube_sz
    };
}

// i know this is disgusting but idk how to make it nicer. TODO: move this into another file so i dont have to look at it
Faces get_face_collisions(Ray crosshair_ray, Block *target_block, Camera *camera) {
    RayCollision collides_top, collides_bottom, collides_left, collides_right, collides_front, collides_back;
    collides_top = GetRayCollisionBox(crosshair_ray,
            (BoundingBox) {
                (Vector3) {
                    target_block->collision_box.min.x,
                    target_block->collision_box.max.y - 0.1,
                    target_block->collision_box.min.z,
                }, target_block->collision_box.max
            });
    collides_bottom = GetRayCollisionBox(crosshair_ray,
            (BoundingBox) {
                target_block->collision_box.min, (Vector3) {
                    target_block->collision_box.max.x,
                    target_block->collision_box.min.y+0.1,
                    target_block->collision_box.max.z,
                }
            });
    collides_back = GetRayCollisionBox(crosshair_ray,
            (BoundingBox) {
                target_block->collision_box.min, (Vector3) {
                    target_block->collision_box.max.x,
                    target_block->collision_box.max.y,
                    target_block->collision_box.min.z+0.1,
                }
            });
    collides_front = GetRayCollisionBox(crosshair_ray,
            (BoundingBox) {
                (Vector3) {
                    target_block->collision_box.min.x,
                    target_block->collision_box.min.y,
                    target_block->collision_box.max.z - 0.1,
                }, target_block->collision_box.max
            });
    collides_left = GetRayCollisionBox(crosshair_ray,
            (BoundingBox) {
                target_block->collision_box.min, (Vector3) {
                    target_block->collision_box.min.x+0.1,
                    target_block->collision_box.max.y,
                    target_block->collision_box.max.z,
                }
            });
    collides_right = GetRayCollisionBox(crosshair_ray,
            (BoundingBox) {
                (Vector3) {
                    target_block->collision_box.max.x - 0.1,
                    target_block->collision_box.min.y,
                    target_block->collision_box.min.z,
                }, target_block->collision_box.max
            });
    RayCollision arr[] = {collides_top, collides_bottom,
                          collides_left, collides_right,
                          collides_front, collides_back};
    float min_dist = -1;
    for (int i = 0; i < sizeof(arr)/sizeof(arr[0]); i++) {
        if (arr[i].distance < min_dist || min_dist == -1)
            min_dist = arr[i].distance;
    }
    if      (collides_top.hit    && min_dist == collides_top.distance)    return FACE_TOP;
    else if (collides_bottom.hit && min_dist == collides_bottom.distance) return FACE_BOTTOM;
    else if (collides_left.hit   && min_dist == collides_left.distance)   return FACE_LEFT;
    else if (collides_right.hit  && min_dist == collides_right.distance)  return FACE_RIGHT;
    else if (collides_front.hit  && min_dist == collides_front.distance)  return FACE_FRONT;
    else if (collides_back.hit   && min_dist == collides_back.distance)   return FACE_BACK;
    else exit(-1);
}

// again this is disgusting, TODO move to another file so i dont have to look at it
void place_on_face(Faces collision_face, Chunk *chunk, Block *target_block, Rectangle *cobblestone_texture) {
    Block *new_block;
    switch (collision_face) {
    case FACE_TOP:
        new_block = &chunk->cubes[(((int)target_block->loc_cube.y+1)*CHUNK_AREA)+((int)target_block->loc_cube.x*CHUNK_WIDTH)+(int)target_block->loc_cube.z];
        new_block->loc_cube = target_block->loc_cube;
        new_block->loc = target_block->loc;
        new_block->loc_cube.y++;
        new_block->loc.y += BLOCK_SIZE;
        break;
    case FACE_BOTTOM:
        new_block = &chunk->cubes[(((int)target_block->loc_cube.y-1)*CHUNK_AREA)+((int)target_block->loc_cube.x*CHUNK_WIDTH)+(int)target_block->loc_cube.z];
        new_block->loc_cube = target_block->loc_cube;
        new_block->loc = target_block->loc;
        new_block->loc_cube.y--;
        new_block->loc.y -= BLOCK_SIZE;
        break;
    case FACE_FRONT:
        new_block = &chunk->cubes[((int)target_block->loc_cube.y*CHUNK_AREA)+((int)target_block->loc_cube.x*CHUNK_WIDTH)+(int)target_block->loc_cube.z+1];
        new_block->loc_cube = target_block->loc_cube;
        new_block->loc = target_block->loc;
        new_block->loc_cube.z++;
        new_block->loc.z += BLOCK_SIZE;
        break;
    case FACE_BACK:
        new_block = &chunk->cubes[((int)target_block->loc_cube.y*CHUNK_AREA)+((int)target_block->loc_cube.x*CHUNK_WIDTH)+(int)target_block->loc_cube.z-1];
        new_block->loc_cube = target_block->loc_cube;
        new_block->loc = target_block->loc;
        new_block->loc_cube.z--;
        new_block->loc.z -= BLOCK_SIZE;
        break;
    case FACE_RIGHT:
        new_block = &chunk->cubes[((int)target_block->loc_cube.y*CHUNK_AREA)+(((int)target_block->loc_cube.x+1)*CHUNK_WIDTH)+(int)target_block->loc_cube.z];
        new_block->loc_cube = target_block->loc_cube;
        new_block->loc = target_block->loc;
        new_block->loc_cube.x++;
        new_block->loc.x += BLOCK_SIZE;
        break;
    case FACE_LEFT:
        new_block = &chunk->cubes[((int)target_block->loc_cube.y*CHUNK_AREA)+(((int)target_block->loc_cube.x-1)*CHUNK_WIDTH)+(int)target_block->loc_cube.z];
        new_block->loc_cube = target_block->loc_cube;
        new_block->loc = target_block->loc;
        new_block->loc_cube.x--;
        new_block->loc.x -= BLOCK_SIZE;
        break;
    default:
        printf("only top face supported as of yet (TODO)\n");
        exit(-1);
    }
    new_block->not_air = true;
    new_block->texture = cobblestone_texture;
    new_block->collision_box = 
        (BoundingBox) {
            (Vector3){new_block->loc.x-BLOCK_SIZE/2, new_block->loc.y-BLOCK_SIZE/2, new_block->loc.z-BLOCK_SIZE/2},
            (Vector3){new_block->loc.x+BLOCK_SIZE/2, new_block->loc.y+BLOCK_SIZE/2, new_block->loc.z+BLOCK_SIZE/2}
        };
}

int main(void) {
    const float screen_scale = 1.5f;
    int screen_width = 800 * screen_scale;
    int screen_height = 450 * screen_scale;

    InitWindow(screen_width, screen_height, "game");

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 10.0f, CHUNK_HEIGHT * 2 + BLOCK_SIZE * 1.0f, 10.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 70.0f; 
    camera.projection = CAMERA_PERSPECTIVE;
    Ray crosshair_ray = { 0 };

    Font font = LoadFontEx("Minecraftia-Regular.ttf", 32, 0, 250);
    Texture2D texture   = LoadTexture("atlas.png");
    Texture2D crosshair = LoadTexture("crosshair.png");
    Texture2D hotbar    = LoadTexture("hotbar.png");
    Texture2D selector  = LoadTexture("hotbar_selector.png");

    Rectangle grass_texture[6] = {0};
    for (int face = 0; face < 6; face++)
        grass_texture[face] = get_texture_rect(texture, 3, 0);
    grass_texture[FACE_TOP] = get_texture_rect(texture, 8, 2);
    grass_texture[FACE_BOTTOM] = get_texture_rect(texture, 2, 0);

    Rectangle dirt_texture[6] = {0};
    for (int face = 0; face < 6; face++)
        dirt_texture[face] = get_texture_rect(texture, 2, 0);
    
    Rectangle cobblestone_texture[6] = {0};
    for (int face = 0; face < 6; face++)
        cobblestone_texture[face] = get_texture_rect(texture, 0, 1);
    (void) cobblestone_texture;
    
    Rectangle bedrock_texture[6] = {0};
    for (int face = 0; face < 6; face++)
        bedrock_texture[face] = get_texture_rect(texture, 1, 1);

    int8_t hotbar_selected = 0; // 0-8
    Chunk *chunk = (Chunk*) malloc(sizeof(Chunk));
    for (int y = 0; y < CHUNK_HEIGHT; y++) {
        Rectangle *texture = (y==CHUNK_HEIGHT-1) ? grass_texture : ((!y) ? bedrock_texture : dirt_texture);
        for (int x = 0; x < CHUNK_WIDTH; x++) {
            for (int z = 0; z < CHUNK_WIDTH; z++) {
                Block *cube = &chunk->cubes[(y*CHUNK_AREA)+(x*CHUNK_WIDTH)+z];
                cube->loc = (Vector3){x*BLOCK_SIZE, y*BLOCK_SIZE, z*BLOCK_SIZE};
                cube->loc_cube = (Vector3){x, y, z};
                cube->texture = texture;
                cube->not_air = true;
                cube->collision_box = 
                    (BoundingBox) {
                        (Vector3){cube->loc.x-BLOCK_SIZE/2, cube->loc.y-BLOCK_SIZE/2, cube->loc.z-BLOCK_SIZE/2},
                        (Vector3){cube->loc.x+BLOCK_SIZE/2, cube->loc.y+BLOCK_SIZE/2, cube->loc.z+BLOCK_SIZE/2}
                    };
            }
        }
    }

    DisableCursor();
    SetTargetFPS(80);

    while (!WindowShouldClose()) {
        UpdateCamera(&camera, CAMERA_FIRST_PERSON);
        UpdateCameraPro(&camera,
            (Vector3){ // wasd
                IsKeyDown(KEY_W)*0.1f - IsKeyDown(KEY_S)*0.1f,
                IsKeyDown(KEY_D)*0.1f - IsKeyDown(KEY_A)*0.1f,
                IsKeyDown(KEY_SPACE)*0.3f - IsKeyDown(KEY_LEFT_SHIFT)*0.3f
            },
            (Vector3){ // mouse movement
                GetMouseDelta().x*0.1f,
                GetMouseDelta().y*0.1f,
                0.0f
            }, 0.0f
        );
        
        hotbar_selected += GetMouseWheelMove();
        for (int i = 0; i < 9; i++) {
            if (IsKeyPressed(KEY_ONE + i))
                hotbar_selected = i;
        }
        if (hotbar_selected > 8)
            hotbar_selected = hotbar_selected % 9;
        if (hotbar_selected < 0)
            hotbar_selected = 9-(-hotbar_selected % 9);
        BeginDrawing();
        ClearBackground((Color){0x78, 0xA7, 0xFF});
        BeginMode3D(camera);

        screen_height = GetScreenHeight();
        screen_width = GetScreenWidth();
        crosshair_ray = GetScreenToWorldRay((Vector2){screen_width/2, screen_height/2}, camera);
        
        int rendered = 0;
        Block *target_block = NULL;
        float target_block_dist = 0;
        for (size_t i = 0; i < MAX_CHUNK_SIZE; i++) {
            // don't render cubes that aren't exposed
            Block this_cube = chunk->cubes[i];
            int x = this_cube.loc_cube.x;
            int y = this_cube.loc_cube.y;
            int z = this_cube.loc_cube.z;
            if (y < MAX_BUILD_HEIGHT-1 && chunk->cubes[((y+1)*CHUNK_AREA)+(x*CHUNK_WIDTH)+z].not_air &&
                y > 0 && chunk->cubes[((y-1)*CHUNK_AREA)+(x*CHUNK_WIDTH)+z].not_air &&
                x > 0 && chunk->cubes[(y*CHUNK_AREA)+((x-1)*CHUNK_WIDTH)+z].not_air &&
                x < CHUNK_WIDTH-1 && chunk->cubes[(y*CHUNK_AREA)+((x+1)*CHUNK_WIDTH)+z].not_air &&
                z > 0 && chunk->cubes[(y*CHUNK_AREA)+(x*CHUNK_WIDTH)+(z-1)].not_air &&
                z < CHUNK_WIDTH-1 && chunk->cubes[(y*CHUNK_AREA)+(x*CHUNK_WIDTH)+(z+1)].not_air) continue;
            rendered++;
            if (!chunk->cubes[i].not_air) continue;
            RayCollision collision = GetRayCollisionBox(crosshair_ray, chunk->cubes[i].collision_box);
            if (collision.hit) {
                float dist = collision.distance;
                if (target_block == NULL || dist < target_block_dist) {
                    target_block = &chunk->cubes[i];
                    target_block_dist = dist;
                }
            }
            DrawCubeTextureRec(texture, chunk->cubes[i].texture, chunk->cubes[i].loc, BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, RAYWHITE);
        }
        if (target_block != NULL && target_block_dist <= 5 * BLOCK_SIZE) {
            DrawCubeWires(target_block->loc, BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, WHITE);     
            if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                target_block->not_air = false;

            if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                // point a ray at each face, check collision, and place on the face it collides with
                // which is closest to the camera
                Faces collision_face = get_face_collisions(crosshair_ray, target_block, &camera);
                place_on_face(collision_face, chunk, target_block, cobblestone_texture);
            }
        }

        EndMode3D();
        const char *text = TextFormat("%i/%i cubes rendered\n"
                                      "FPS: %i\n"
                                      "XYZ: %.0f,%.0f,%.0f\n"
                                      "Target: %.0f,%.0f,%.0f\n",
                                        rendered, CHUNK_SIZE, GetFPS(),
                                        camera.position.x/2, camera.position.y/2, camera.position.z/2,
                                        camera.target.x/2, camera.target.y/2, camera.target.z/2);

        DrawTextEx(font, text, (Vector2){9, 11}, 24, 1, GRAY);
        DrawTextEx(font, text, (Vector2){8, 12}, 24, 1, GRAY);
        DrawTextEx(font, text, (Vector2){10, 10},  24, 1, RAYWHITE);
       
        // crosshair
        DrawTextureEx(crosshair,
                (Vector2){screen_width/2-crosshair.width*0.05/2,
                          screen_height/2-crosshair.height*0.05/2},
                0, 0.05, GRAY);
        
        DrawTextureEx(hotbar, 
                (Vector2){screen_width/2-hotbar.width*1.5f/2,
                          screen_height-hotbar.height*1.5f-10},
                0, 1.5f, WHITE);
        DrawTextureEx(selector, 
                (Vector2){screen_width/2-hotbar.width*1.5f/2 + hotbar_selected * selector.width*2.48f - 1,
                          screen_height-hotbar.height*1.5f-11},
                0, 3, WHITE);

        EndDrawing();
    }

    free(chunk);
    CloseWindow();
    return 0;
}
