#include "game.h"

Block *get_block(Chunk *chunk, long x, long y, long z) {
	if (x < 0 || x >= CHUNK_WIDTH) return NULL;
	if (y < 0 || y >= CHUNK_HEIGHT) return NULL;
	if (z < 0 || z >= CHUNK_WIDTH) return NULL;
	return &chunk->cubes[(y*CHUNK_AREA)+(x*CHUNK_WIDTH)+z];
}

Chunk *get_chunk(long x, long z) {
	return NULL;
}
