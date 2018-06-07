// (c) 2016 - Present, Bracket Productions

#include "chunking.h"

int chunk_id_by_world_pos(const int &x, const int &y, const int &z) noexcept
{
	using namespace nfu;
	const int chunk_x = x / CHUNK_SIZE;
	const int chunk_y = y / CHUNK_SIZE;
	const int chunk_z = z / CHUNK_SIZE;
	return chunk_idx(chunk_x, chunk_y, chunk_z);
}
