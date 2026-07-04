#include <stdlib.h>
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "game.h"

static bool check_air(Chunk *chunk, long x, long y, long z) {
	Block *block = get_block_in_chunk(chunk, x, y, z);
	if (!block) {
		// TODO : check other chunk
		return true;
	}
	return block->type == BLOCK_AIR;
}

static void add_face(Mesh *mesh, Face face, Vector3 pos, Vector2 *uv1, Vector2 *uv2) {
	Vector3 normal;
	Vector3 v[4];
	switch (face) {
	case FACE_LEFT:
		normal = (Vector3){-1.0f, 0.0f, 0.0f};
		v[0] = (Vector3){0.0f, 0.0f, 0.0f};
		v[1] = (Vector3){0.0f, 0.0f, 1.0f};
		v[2] = (Vector3){0.0f, 1.0f, 1.0f};
		v[3] = (Vector3){0.0f, 1.0f, 0.0f};
		break;
	case FACE_RIGHT:
		normal = (Vector3){ 1.0f, 0.0f, 0.0f};
		v[0] = (Vector3){1.0f, 0.0f, 1.0f};
		v[1] = (Vector3){1.0f, 0.0f, 0.0f};
		v[2] = (Vector3){1.0f, 1.0f, 0.0f};
		v[3] = (Vector3){1.0f, 1.0f, 1.0f};
		break;
	case FACE_BOTTOM:
		normal = (Vector3){0.0f, -1.0f, 0.0f};
		v[0] = (Vector3){0.0f, 0.0f, 1.0f};
		v[1] = (Vector3){0.0f, 0.0f, 0.0f};
		v[2] = (Vector3){1.0f, 0.0f, 0.0f};
		v[3] = (Vector3){1.0f, 0.0f, 1.0f};
		break;
	case FACE_TOP:
		normal = (Vector3){0.0f,  1.0f, 0.0f};
		v[0] = (Vector3){0.0f, 1.0f, 0.0f};
		v[1] = (Vector3){0.0f, 1.0f, 1.0f};
		v[2] = (Vector3){1.0f, 1.0f, 1.0f};
		v[3] = (Vector3){1.0f, 1.0f, 0.0f};
		break;
	case FACE_BACK:
		normal = (Vector3){0.0f, 0.0f, -1.0f};
		v[0] = (Vector3){1.0f, 0.0f, 0.0f};
		v[1] = (Vector3){0.0f, 0.0f, 0.0f};
		v[2] = (Vector3){0.0f, 1.0f, 0.0f};
		v[3] = (Vector3){1.0f, 1.0f, 0.0f};
		break;
	case FACE_FRONT:
		normal = (Vector3){0.0f, 0.0f,  1.0f};
		v[0] = (Vector3){0.0f, 0.0f, 1.0f};
		v[1] = (Vector3){1.0f, 0.0f, 1.0f};
		v[2] = (Vector3){1.0f, 1.0f, 1.0f};
		v[3] = (Vector3){0.0f, 1.0f, 1.0f};
		break;
	case FACE_NONE:
		return;
	}

	Vector2 uvs[4] = {
		(Vector2){uv1->x, uv2->y},
		(Vector2){uv2->x, uv2->y},
		(Vector2){uv2->x, uv1->y},
		(Vector2){uv1->x, uv1->y},
	};

	size_t v_index = mesh->vertexCount;
	size_t t_index = mesh->triangleCount;
	mesh->vertexCount += 4;
	mesh->triangleCount += 2;
	mesh->vertices  = realloc(mesh->vertices , mesh->vertexCount * 3 * sizeof(float));
	mesh->normals   = realloc(mesh->normals  , mesh->vertexCount * 3 * sizeof(float));
	mesh->texcoords = realloc(mesh->texcoords, mesh->vertexCount * 2 * sizeof(float));
	mesh->indices = realloc(mesh->indices, mesh->triangleCount * 3 * sizeof(unsigned short));

	// add vertices
	for (int i=0; i<4; i++) {
		mesh->vertices[(v_index + i) * 3 + 0] = v[i].x * BLOCK_SIZE + pos.x;
		mesh->vertices[(v_index + i) * 3 + 1] = v[i].y * BLOCK_SIZE + pos.y;
		mesh->vertices[(v_index + i) * 3 + 2] = v[i].z * BLOCK_SIZE + pos.z;

		mesh->normals[(v_index + i) * 3 + 0] = normal.x;
		mesh->normals[(v_index + i) * 3 + 1] = normal.y;
		mesh->normals[(v_index + i) * 3 + 2] = normal.z;

		mesh->texcoords[(v_index + i) * 2 + 0] = uvs[i].x;
		mesh->texcoords[(v_index + i) * 2 + 1] = uvs[i].y;
	}

	mesh->indices[t_index * 3 + 0] = v_index + 0;
	mesh->indices[t_index * 3 + 1] = v_index + 1;
	mesh->indices[t_index * 3 + 2] = v_index + 2;
	mesh->indices[t_index * 3 + 3] = v_index + 0;
	mesh->indices[t_index * 3 + 4] = v_index + 2;
	mesh->indices[t_index * 3 + 5] = v_index + 3;
}

static void generate_block_mesh(Chunk *chunk, Block *block, long x, long y, long z){
    if (block->type == BLOCK_AIR) return;
    Texture2D *texture = &block_material.maps[MATERIAL_MAP_DIFFUSE].texture;
    float tex_width = (float)texture->width;
    float tex_height = (float)texture->height;

    bool sides_air[6];
    sides_air[FACE_LEFT]    = check_air(chunk, x-1, y, z);
    sides_air[FACE_RIGHT]   = check_air(chunk, x+1, y, z);
    sides_air[FACE_BOTTOM]  = check_air(chunk, x, y-1, z);
    sides_air[FACE_TOP]     = check_air(chunk, x, y+1, z);
    sides_air[FACE_BACK]    = check_air(chunk, x, y, z-1);
    sides_air[FACE_FRONT]   = check_air(chunk, x, y, z+1);

    Vector3 position = {x * BLOCK_SIZE, y * BLOCK_SIZE, z * BLOCK_SIZE};

    for (int i=0; i<6; i++) {
    	if (sides_air[i]) {
	    // We calculate the normalized texture coordinates for the desired texture-source-rectangle
	    // It means converting from (tex.width, tex.height) coordinates to [0.0f, 1.0f] equivalent
	    Vector2 uv1 = {
		texture_uvs[block->type][i].x / tex_width,
	        texture_uvs[block->type][i].y / tex_height,
	    };
	    Vector2 uv2 = {
		(texture_uvs[block->type][i].x + texture_uvs[block->type][i].width) / tex_width,
	        (texture_uvs[block->type][i].y + texture_uvs[block->type][i].height) / tex_height,
	    };
	    add_face(&chunk->mesh, i, position, &uv1, &uv2);
	}
    }
}

static void generate_chunk_mesh(Chunk *chunk) {
	printf("regenerate mesh for chunk %ld %ld\n", chunk->x, chunk->z);
	UnloadMesh(chunk->mesh);
	chunk->mesh.vertexCount = 0;
	chunk->mesh.triangleCount = 0;
	for (int z=0; z<CHUNK_WIDTH; z++) {
		for (int x=0; x<CHUNK_WIDTH; x++) {
			for (int y=0; y<CHUNK_HEIGHT; y++) {
				Block *block = get_block_in_chunk(chunk, x, y, z);
				generate_block_mesh(chunk, block, x, y, z);
			}
		}
	}
	UploadMesh(&chunk->mesh, false);
	chunk->flags &= ~CHUNK_DIRTY;
	return;
}

static Mesh *get_chunk_mesh(Chunk *chunk) {
	if (chunk->flags & CHUNK_DIRTY) {
		generate_chunk_mesh(chunk);
	}
	return &chunk->mesh;
}

void draw_chunk(Chunk *chunk) {
    Mesh *mesh = get_chunk_mesh(chunk);
    // TODO : apply position ?
    Matrix transform = MatrixTranslate(
	chunk->x * CHUNK_WIDTH * BLOCK_SIZE,
	0.0f,
	chunk->z * CHUNK_WIDTH * BLOCK_SIZE
    );
    DrawMesh(*mesh, block_material, transform);
}
