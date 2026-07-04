// borrowed from https://github.com/raysan5/raylib/blob/master/examples with modifications

#include "raylib.h"
#include "rlgl.h"
#include "game.h"

// Draw cube with a different texture piece applied to all faces, and returns number of faces drawn
uint8_t DrawCubeTextureRec(Texture2D texture, Rectangle sources[6], Vector3 position, float width, float height, float length,
                                                                                                    Color color, bool *sides_not_air){
    float x = position.x;
    float y = position.y;
    float z = position.z;
    float texWidth = (float)texture.width;
    float texHeight = (float)texture.height;
    rlSetTexture(texture.id);
    uint8_t ret = 0;

    // Set desired texture to be enabled while drawing following vertex data

    // We calculate the normalized texture coordinates for the desired texture-source-rectangle
    // It means converting from (tex.width, tex.height) coordinates to [0.0f, 1.0f] equivalent
    rlBegin(RL_QUADS);
        rlColor4ub(color.r, color.g, color.b, color.a);

        // Front face
        Rectangle source = sources[0];
        if (!sides_not_air[FACE_FRONT]) {
            rlNormal3f(0.0f, 0.0f, 1.0f);
            rlTexCoord2f(source.x/texWidth, (source.y + source.height)/texHeight);
            rlVertex3f(x - width/2, y - height/2, z + length/2);
            rlTexCoord2f((source.x + source.width)/texWidth, (source.y + source.height)/texHeight);
            rlVertex3f(x + width/2, y - height/2, z + length/2);
            rlTexCoord2f((source.x + source.width)/texWidth, source.y/texHeight);
            rlVertex3f(x + width/2, y + height/2, z + length/2);
            rlTexCoord2f(source.x/texWidth, source.y/texHeight);
            rlVertex3f(x - width/2, y + height/2, z + length/2);
            ret++;
        }

        // Back face
        if (!sides_not_air[FACE_BACK]) {
            source = sources[1];
            rlNormal3f(0.0f, 0.0f, - 1.0f);
            rlTexCoord2f((source.x + source.width)/texWidth, (source.y + source.height)/texHeight);
            rlVertex3f(x - width/2, y - height/2, z - length/2);
            rlTexCoord2f((source.x + source.width)/texWidth, source.y/texHeight);
            rlVertex3f(x - width/2, y + height/2, z - length/2);
            rlTexCoord2f(source.x/texWidth, source.y/texHeight);
            rlVertex3f(x + width/2, y + height/2, z - length/2);
            rlTexCoord2f(source.x/texWidth, (source.y + source.height)/texHeight);
            rlVertex3f(x + width/2, y - height/2, z - length/2);
            ret++;
        }

        // Top face
        if (!sides_not_air[FACE_TOP]) {
            source = sources[2];
            rlNormal3f(0.0f, 1.0f, 0.0f);
            rlTexCoord2f(source.x/texWidth, source.y/texHeight);
            rlVertex3f(x - width/2, y + height/2, z - length/2);
            rlTexCoord2f(source.x/texWidth, (source.y + source.height)/texHeight);
            rlVertex3f(x - width/2, y + height/2, z + length/2);
            rlTexCoord2f((source.x + source.width)/texWidth, (source.y + source.height)/texHeight);
            rlVertex3f(x + width/2, y + height/2, z + length/2);
            rlTexCoord2f((source.x + source.width)/texWidth, source.y/texHeight);
            rlVertex3f(x + width/2, y + height/2, z - length/2);
            ret++;
        }

        // Bottom face
        if (!sides_not_air[FACE_BOTTOM]) {
            source = sources[3];
            rlNormal3f(0.0f, - 1.0f, 0.0f);
            rlTexCoord2f((source.x + source.width)/texWidth, source.y/texHeight);
            rlVertex3f(x - width/2, y - height/2, z - length/2);
            rlTexCoord2f(source.x/texWidth, source.y/texHeight);
            rlVertex3f(x + width/2, y - height/2, z - length/2);
            rlTexCoord2f(source.x/texWidth, (source.y + source.height)/texHeight);
            rlVertex3f(x + width/2, y - height/2, z + length/2);
            rlTexCoord2f((source.x + source.width)/texWidth, (source.y + source.height)/texHeight);
            rlVertex3f(x - width/2, y - height/2, z + length/2);
            ret++;
        }

        // Right face
        if (!sides_not_air[FACE_RIGHT]) {
            source = sources[4];
            rlNormal3f(1.0f, 0.0f, 0.0f);
            rlTexCoord2f((source.x + source.width)/texWidth, (source.y + source.height)/texHeight);
            rlVertex3f(x + width/2, y - height/2, z - length/2);
            rlTexCoord2f((source.x + source.width)/texWidth, source.y/texHeight);
            rlVertex3f(x + width/2, y + height/2, z - length/2);
            rlTexCoord2f(source.x/texWidth, source.y/texHeight);
            rlVertex3f(x + width/2, y + height/2, z + length/2);
            rlTexCoord2f(source.x/texWidth, (source.y + source.height)/texHeight);
            rlVertex3f(x + width/2, y - height/2, z + length/2);
            ret++;
        }

        // Left face
        if (!sides_not_air[FACE_LEFT]) {
            source = sources[5];
            rlNormal3f( - 1.0f, 0.0f, 0.0f);
            rlTexCoord2f(source.x/texWidth, (source.y + source.height)/texHeight);
            rlVertex3f(x - width/2, y - height/2, z - length/2);
            rlTexCoord2f((source.x + source.width)/texWidth, (source.y + source.height)/texHeight);
            rlVertex3f(x - width/2, y - height/2, z + length/2);
            rlTexCoord2f((source.x + source.width)/texWidth, source.y/texHeight);
            rlVertex3f(x - width/2, y + height/2, z + length/2);
            rlTexCoord2f(source.x/texWidth, source.y/texHeight);
            rlVertex3f(x - width/2, y + height/2, z - length/2);
            ret++;
        }

    rlEnd();

    rlSetTexture(0);
    return ret;
}
