// (c) 2016 - Present, Bracket Productions

#pragma once

#include "CoreMinimal.h"

namespace BlockType {
	constexpr uint8_t MAX_BLOCK_TYPE = 9;
	constexpr uint8_t NONE = 0;
	constexpr uint8_t WATER = 1;
	constexpr uint8_t PLAINS = 2;
	constexpr uint8_t HILLS = 3;
	constexpr uint8_t MOUNTAINS = 4;
	constexpr uint8_t MARSH = 5;
	constexpr uint8_t PLATEAU = 6;
	constexpr uint8_t HIGHLANDS = 7;
	constexpr uint8_t COASTAL = 8;
	constexpr uint8_t SALT_MARSH = 9;
}

namespace RiverMaskBits {
	constexpr uint8 NONE = 0;
	constexpr uint8 NORTH = 1;
	constexpr uint8 EAST = 2;
	constexpr uint8 SOUTH = 4;
	constexpr uint8 WEST = 8;
	constexpr uint8 START = 16;
}

namespace FeatureMaskBits {
	constexpr uint32 HUTS = 1;
	constexpr uint32 ROAD_N = 2;
	constexpr uint32 ROAD_E = 4;
	constexpr uint32 ROAD_S = 8;
	constexpr uint32 ROAD_W = 16;
	constexpr uint32 FARM = 32;
	constexpr uint32 LUMBER = 64;
	constexpr uint32 MINE = 128;
	constexpr uint32 PALLISADE = 256;
}

namespace nfu {

	constexpr int WORLD_HEIGHT = 128;
	constexpr int WORLD_WIDTH = 128;
	constexpr int WORLD_TILES_COUNT = WORLD_HEIGHT * WORLD_WIDTH;

	constexpr int REGION_WIDTH = 256;
	constexpr int REGION_HEIGHT = 256;
	constexpr int REGION_DEPTH = 128;
	constexpr int REGION_TILES_COUNT = REGION_WIDTH * REGION_HEIGHT * REGION_DEPTH;

	constexpr float NOISE_SIZE = 768.0f;

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

