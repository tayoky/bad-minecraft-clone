#include "raylib.h"
#include "rlgl.h"
#include "game.h"
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

void place_on_face(Faces collision_face, Chunk *chunk, Block *target_block, Rectangle *texture);
Faces get_face_collisions(Ray crosshair_ray, Block *target_block, Camera *camera);
uint8_t DrawCubeTextureRec(Texture2D texture, Rectangle *source, Vector3 position, float width, float height, float length, Color color, bool *sides_not_air);

Rectangle get_texture_rect(Texture2D texture, int x, int y) {
    int cube_sz = texture.width/16;
    return (Rectangle){ cube_sz * x,
                        cube_sz * y,
                        cube_sz,
                        cube_sz
    };
}

// for blocks with all 6 sides with the same texture
void load_basic_block_texture(Texture atlas, int atlas_x, int atlas_y, Rectangle *buf) {
    for (int face = 0; face < 6; face++)
        buf[face] = get_texture_rect(atlas, atlas_x, atlas_y);
}

int main(void) {
    const float screen_scale = 1.5f;
    int screen_width = 800 * screen_scale;
    int screen_height = 450 * screen_scale;

    InitWindow(screen_width, screen_height, "game");

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 10.0f, CHUNK_HEIGHT * BLOCK_SIZE + BLOCK_SIZE * 1.0f, 10.0f };
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

    // hotbar textures: stone, dirt, oak planks, cobblestone, sand, gravel, oak logs, bricks, glowstone
    Rectangle dirt_texture[6], cobblestone_texture[6], bedrock_texture[6], grass_texture[6], glowstone_texture[6], 
              stone_texture[6], oak_planks_texture[6], sand_texture[6], gravel_texture[6], oak_log_texture[6], bricks_texture[6];
    load_basic_block_texture(texture, 3, 0, grass_texture);
    load_basic_block_texture(texture, 2, 0, dirt_texture);
    load_basic_block_texture(texture, 0, 1, cobblestone_texture);
    load_basic_block_texture(texture, 1, 1, bedrock_texture);
    load_basic_block_texture(texture, 9, 6, glowstone_texture);
    load_basic_block_texture(texture, 1, 0, stone_texture);
    load_basic_block_texture(texture, 4, 0, oak_planks_texture);
    load_basic_block_texture(texture, 2, 1, sand_texture);
    load_basic_block_texture(texture, 3, 1, gravel_texture);
    load_basic_block_texture(texture, 4, 1, oak_log_texture);
    load_basic_block_texture(texture, 7, 0, bricks_texture);
    
    grass_texture[FACE_TOP] = get_texture_rect(texture, 8, 2);
    grass_texture[FACE_BOTTOM] = get_texture_rect(texture, 2, 0);

    oak_log_texture[FACE_TOP] = oak_log_texture[FACE_BOTTOM] = get_texture_rect(texture, 5, 1);
    
    int8_t hotbar_selected = 0; // 0-8
    Rectangle *hotbar_slots[9] = {
        stone_texture, dirt_texture, oak_planks_texture, cobblestone_texture,
        sand_texture, gravel_texture, oak_log_texture, bricks_texture, glowstone_texture
    };

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

    int num_faces = CHUNK_SIZE * 6;
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
            bool sides_not_air[6] = {0};
            sides_not_air[FACE_FRONT ] = z < CHUNK_WIDTH-1 && chunk->cubes[(y*CHUNK_AREA)+(x*CHUNK_WIDTH)+(z+1)].not_air;
            sides_not_air[FACE_BACK  ] = z > 0 && chunk->cubes[(y*CHUNK_AREA)+(x*CHUNK_WIDTH)+(z-1)].not_air;
            sides_not_air[FACE_RIGHT ] = x < CHUNK_WIDTH-1 && chunk->cubes[(y*CHUNK_AREA)+((x+1)*CHUNK_WIDTH)+z].not_air;
            sides_not_air[FACE_LEFT  ] = x > 0 && chunk->cubes[(y*CHUNK_AREA)+((x-1)*CHUNK_WIDTH)+z].not_air;
            sides_not_air[FACE_BOTTOM] = y > 0 && chunk->cubes[((y-1)*CHUNK_AREA)+(x*CHUNK_WIDTH)+z].not_air;
            sides_not_air[FACE_TOP   ] = y < MAX_BUILD_HEIGHT-1 && chunk->cubes[((y+1)*CHUNK_AREA)+(x*CHUNK_WIDTH)+z].not_air;
            bool face_is_visible = false;
            for (uint8_t face = 0; face < FACE_NONE; face++) {
                if (!sides_not_air[face]) continue;
                face_is_visible = true;
                break;
            }
            if (!chunk->cubes[i].not_air || !face_is_visible) continue;
            RayCollision collision = GetRayCollisionBox(crosshair_ray, chunk->cubes[i].collision_box);
            if (collision.hit) {
                float dist = collision.distance;
                if (target_block == NULL || dist < target_block_dist) {
                    target_block = &chunk->cubes[i];
                    target_block_dist = dist;
                }
            }
            rendered += DrawCubeTextureRec(texture, chunk->cubes[i].texture, chunk->cubes[i].loc,
                    BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, WHITE, sides_not_air);
        }
        if (target_block != NULL && target_block_dist <= 5 * BLOCK_SIZE) {
            DrawCubeWires(target_block->loc, BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, WHITE);     
            if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                num_faces -= 6;
                target_block->not_air = false;
            }

            if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && hotbar_slots[hotbar_selected] != NULL) {
                // point a ray at each face, check collision, and place on the face it collides with
                // which is closest to the camera
                Faces collision_face = get_face_collisions(crosshair_ray, target_block, &camera);
                place_on_face(collision_face, chunk, target_block, hotbar_slots[hotbar_selected]);
                num_faces += 6;
            }
        }

        EndMode3D();
        const char *text = TextFormat("%i/%i faces rendered (%.1f%)\n"
                                      "FPS: %i\n"
                                      "XYZ: %.0f,%.0f,%.0f\n",
                                        rendered, num_faces, ((float)rendered)/((float)num_faces)*100, GetFPS(),
                                        camera.position.x/2, camera.position.y/2, camera.position.z/2);

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
        for (int i = 0; i < 9; i++) {
            if (hotbar_slots[i] == NULL) continue;
            Rectangle dest_rec = {screen_width/2-hotbar.width*1.5f/2 + i * selector.width*2.5f - 1 + 18, // x
                                 screen_height-hotbar.height*1.5f-11 + 18, // y
                                 35, 35};
            DrawTexturePro(texture, *hotbar_slots[i], dest_rec, (Vector2){0, 0}, 0.0f, WHITE);
        }
        DrawBillboard(camera, hotbar, (Vector3) {0,CHUNK_HEIGHT*2+5,0}, 1, WHITE); // Draw a billboard texture
        EndDrawing();
    }

    free(chunk);
    UnloadTexture(texture);
    UnloadTexture(crosshair);
    UnloadTexture(hotbar);
    UnloadTexture(selector);
    UnloadFont(font);
    CloseWindow();
    return 0;
}
