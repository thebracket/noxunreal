// (c) 2016 - Present, Bracket Productions

#pragma once

#include "Runtime/Core/Public/Containers/Array.h"
#include "Runtime/Core/Public/Containers/Map.h"
#include "regiondefs.h"
#include "constants.h"

int chunk_id_by_world_pos(const int &x, const int &y, const int &z) noexcept;

struct cube_t {
	int x, y, z, w, h, d;
	unsigned int tex;
};

struct floor_t {
	int x, y, z, w, h;
	unsigned int tex;
};

struct layer_t {
	TArray<cube_t> cubes;
	TArray<floor_t> floors;
	TArray<floor_t> design_mode;
};

struct chunk_t {
	int index = 0, base_x = 0, base_y = 0, base_z = 0;
	TArray<layer_t> layers;
	TMap<int, TArray<TTuple<int, int, int>>> static_voxel_models;
	TArray<TTuple<int, int, int, int, int>> vegetation_models; // plant, state, x, y, z
};
