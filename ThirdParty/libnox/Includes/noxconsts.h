#ifndef NOXCONSTS_H
#define NOXCONSTS_H

namespace nf {

	constexpr int WORLD_HEIGHT = 128;
	constexpr int WORLD_WIDTH = 128;
	constexpr int WORLD_TILES_COUNT = WORLD_HEIGHT * WORLD_WIDTH;

	constexpr int REGION_WIDTH = 256;
	constexpr int REGION_HEIGHT = 256;
	constexpr int REGION_DEPTH = 128;
	constexpr int REGION_TILES_COUNT = REGION_WIDTH * REGION_HEIGHT * REGION_DEPTH;

	constexpr int CHUNK_SIZE = 64;
	constexpr int CHUNK_WIDTH = REGION_WIDTH / CHUNK_SIZE;
	constexpr int CHUNK_HEIGHT = REGION_HEIGHT / CHUNK_SIZE;
	constexpr int CHUNK_DEPTH = REGION_DEPTH / CHUNK_SIZE;
	constexpr int CHUNK_TOTAL = CHUNK_HEIGHT * CHUNK_WIDTH;
	constexpr int CHUNKS_TOTAL = CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH;

	inline int chunk_idx(const int chunk_x, const int &chunk_y, const int &chunk_z) noexcept {
		return (chunk_z * CHUNK_HEIGHT * CHUNK_WIDTH) + (chunk_y * CHUNK_WIDTH) + chunk_x;
	}

}

#endif