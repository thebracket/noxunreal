// (c) 2016 - Present, Bracket Productions

#include "NRegion.h"
#include "../Public/NoxGameInstance.h"
#include "../Raws/NRaws.h"
#include "../BEngine/BECS.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "../BEngine/RexReader.h"

struct region_water_feature_tile {
	region_water_feature_tile() = default;
	region_water_feature_tile(const int IDX, const bool hasw, const int d, const int a) noexcept : idx(IDX), has_water_already(hasw), depth(d), altitude(a) {}
	int idx;
	bool has_water_already = false;
	int depth = 0;
	int altitude = nfu::REGION_DEPTH;
};

template <typename CONTENTS>
void RegionInitialize(TArray<CONTENTS> &container, const CONTENTS Initial) {
	container.Empty();
	container.AddUninitialized(nfu::REGION_TILES_COUNT);
	for (size_t i = 0; i < nfu::REGION_TILES_COUNT; ++i) {
		container[i] = Initial;
	}
}

void UNRegion::SetupEmptyRegion() {
	RegionInitialize<uint8>(TileType, 0);
	RegionInitialize<uint16>(TileMaterial, 0);
	RegionInitialize<uint16>(HitPoints, 0);
	RegionInitialize<uint8>(VegetationHitPoints, 0);
	RegionInitialize<uint32>(BuildingId, 0);
	RegionInitialize<uint32>(TreeId, 0);
	RegionInitialize<uint32>(BridgeId, 0);
	RegionInitialize<uint16>(TileVegetationType, 0);
	RegionInitialize<uint16>(TileVegetationTicker, 0);
	RegionInitialize<uint8>(TileVegetationLifecycle, 0);
	RegionInitialize<uint32>(StockpileId, 0);
	RegionInitialize<uint32>(TileFlags, 0);
	RegionInitialize<uint32>(WaterLevel, 0);
}

void UNRegion::BuildRegion(const int StartX, const int StartY, const bool StartingArea) {
	using namespace nfu;

	// New Region
	SetupEmptyRegion();
	RegionX = StartX;
	RegionY = StartY;
	RandomNumberGenerator * rng = planet->GetRNG();

	// Lookup Biome
	BiomeIndex = planet->Landblocks[planet->idx(RegionX, RegionY)].biome_idx;
	const auto Biome = &planet->Biomes[BiomeIndex];

	// Build region height map
	auto HeightMap = CreateEmptyHeightmap();
	BuildHeightmapFromNoise(HeightMap);

	// Wet places
	auto PooledWater = CreateEmptyHeightmap();
	TArray<TPair<int, uint8>> WaterSpawners;

	// Sub-biome map
	auto SubRegions = CreateSubregions(HeightMap, rng, PooledWater, WaterSpawners);

	// Hydrology
	JustAddWater(HeightMap, rng, PooledWater, WaterSpawners);

	// Strata
	auto strata = BuildStrata(HeightMap, rng);
	LayStrata(HeightMap, rng, PooledWater, WaterSpawners, strata);

	// Build ramps and beaches
	BuildRamps(PooledWater);
	BuildBeaches();

	// Determine Crash Site
	const int crash_x = StartingArea ? REGION_WIDTH / 2 : 1;
	const int crash_y = StartingArea ? REGION_HEIGHT / 2 : 1;
	const int crash_z = GroundZ(crash_x, crash_y);

	// Build game components
	if (StartingArea) BuildGameComponents(crash_x, crash_y, crash_z);
	
	// Trees and Blight
	// TODO: Determine blight level
	const int blight_level = 0;
	if (blight_level < 100) {
		BuildTrees(rng, StartingArea);
	}

	// Debris Trail
	if (StartingArea) BuildDebrisTrail(crash_x, crash_y);

	// Build Escape Pod
	if (StartingArea) BuildEscapePod(crash_x, crash_y, crash_z);

	// Add Settlers
	if (StartingArea) {
		TArray<TTuple<int, int, int>> settler_spawn_points;
		settler_spawn_points.Emplace(TTuple<int,int,int>(crash_x - 3, crash_y - 2, crash_z + 1));
		settler_spawn_points.Emplace(TTuple<int, int, int>(crash_x - 2, crash_y - 2, crash_z + 1));
		settler_spawn_points.Emplace(TTuple<int, int, int>(crash_x - 1, crash_y - 2, crash_z + 1));
		settler_spawn_points.Emplace(TTuple<int, int, int>(crash_x, crash_y - 2, crash_z + 1));
		settler_spawn_points.Emplace(TTuple<int, int, int>(crash_x + 1, crash_y - 2, crash_z + 1));
		settler_spawn_points.Emplace(TTuple<int, int, int>(crash_x - 3, crash_y, crash_z + 1));
		settler_spawn_points.Emplace(TTuple<int, int, int>(crash_x - 2, crash_y, crash_z + 1));
		settler_spawn_points.Emplace(TTuple<int, int, int>(crash_x - 1, crash_y, crash_z + 1));
		settler_spawn_points.Emplace(TTuple<int, int, int>(crash_x, crash_y, crash_z + 1));
		settler_spawn_points.Emplace(TTuple<int, int, int>(crash_x + 1, crash_y, crash_z + 1));

		for (int i = 0; i<planet->StartingSettlers; ++i) {
			auto sx = settler_spawn_points[i % settler_spawn_points.Num()].Get<0>();
			auto sy = settler_spawn_points[i % settler_spawn_points.Num()].Get<1>();
			auto sz = settler_spawn_points[i % settler_spawn_points.Num()].Get<2>();
			CreateSettler(sx, sy, sz, rng, i % 3);
		}
	}

	// Add Features

	// Recalculate all tiles
	TileRecalcAll();

	// Save it
}

TArray<uint8> UNRegion::CreateEmptyHeightmap() {
	TArray<uint8> HeightMap;
	RegionInitialize<uint8>(HeightMap, 0);
	return HeightMap;
}

namespace region_impl {

	inline float noise_x(const int world_x, const int region_x) {
		const auto big_x = static_cast<float>((world_x * nfu::WORLD_WIDTH) + region_x);
		return (big_x / (static_cast<float>(nfu::WORLD_WIDTH) * static_cast<float>(nfu::REGION_WIDTH))) * nfu::NOISE_SIZE;
	}

	inline float noise_y(const int world_y, const int region_y) {
		const auto big_y = static_cast<float>((world_y * nfu::WORLD_HEIGHT) + region_y);
		return (big_y / (static_cast<float>(nfu::WORLD_HEIGHT) * static_cast<float>(nfu::REGION_HEIGHT))) * nfu::NOISE_SIZE;
	}

	inline uint8_t noise_to_planet_height(const float &n) {
		return static_cast<uint8_t>((n + 1.0F) * 150.0F);
	}
}

void UNRegion::BuildHeightmapFromNoise(TArray<uint8> &HeightMap) {
	auto noise = planet->GetNoiseMap();

	for (auto y = 0; y<nfu::REGION_HEIGHT; ++y) {
		for (auto x = 0; x<nfu::REGION_WIDTH; ++x) {
			const auto nx = region_impl::noise_x(RegionX, x);
			const auto ny = region_impl::noise_y(RegionY, y);
			const auto nh = noise.GetNoise(nx, ny);
			const auto altitude = region_impl::noise_to_planet_height(nh);
			const auto cell_idx = (y * nfu::REGION_WIDTH) + x;
			HeightMap[cell_idx] = altitude - planet->water_height + 5;
		}
	}
}

TArray<int32> UNRegion::CreateSubregions(TArray<uint8> &HeightMap, RandomNumberGenerator * rng, TArray<uint8> &PooledWater, TArray<TPair<int, uint8>> &WaterSpawners) {
	using namespace nfu;

	const int region_variance = planet->Landblocks[planet->idx(RegionX, RegionY)].variance;
	const int n_subregions = 64 + rng->RollDice(1, 20) + (region_variance * 4);

	// Finding sub biomes
	TArray<TPair<int, int>> centroids;
	centroids.AddUninitialized(n_subregions);
	for (std::size_t i = 0; i<n_subregions; ++i) {
		centroids[i] = TPair<int,int>(rng->RollDice(1, REGION_WIDTH) - 1, rng->RollDice(1, REGION_HEIGHT) - 1);
	}
	TArray<int> subregion_idx;
	subregion_idx.AddUninitialized(REGION_HEIGHT * REGION_WIDTH);

	for (int y = 0; y<REGION_HEIGHT; ++y) {
		for (int x = 0; x<REGION_WIDTH; ++x) {
			float distance = 20000.0;
			int sub_idx = -1;
			int i = 0;
			for (const auto &centroid : centroids) {
				const float D = distance2d_squared(x, y, centroid.Key, centroid.Value);
				if (D < distance) {
					distance = D;
					sub_idx = i;
				}
				++i;
			}
			subregion_idx[(y*REGION_WIDTH) + x] = sub_idx;

		}
	}

	// Making sub-biomes
	TArray<int> variance;
	for (int i = 0; i<n_subregions; ++i) {
		const int up_variance = rng->RollDice(1, 2) - 1;
		const int down_variance = rng->RollDice(1, 2) - 1;
		int amount = up_variance - down_variance;
		// Disable murky pools for now
		//if (rng.roll_dice(1,100000) < rainfall) amount = -10;
		variance.Emplace(amount);
	}

	// Applying sub-biomes
	// TODO: Actual variance rather than just altitude when the raws permit it
	for (int y = 0; y<REGION_HEIGHT; ++y) {
		for (int x = 0; x<REGION_WIDTH; ++x) {
			const int tile_idx = (y * REGION_WIDTH) + x;
			const int sub_idx = subregion_idx[tile_idx];
			const int delta_z = variance[sub_idx];
			if (distance2d(x, y, REGION_WIDTH / 2, REGION_HEIGHT / 2) > 20) {
				if (delta_z == -10) {
					// Murky Pool
					PooledWater[tile_idx] = 10;
					HeightMap[tile_idx] -= 2;
					WaterSpawners.Emplace(TPair<int,uint8>( 1, tile_idx )); // Murky pool spawner
				}
				else {
					HeightMap[tile_idx] += delta_z;
				}
			}
			else {
				HeightMap[tile_idx] = HeightMap[(REGION_HEIGHT / 2 * REGION_WIDTH) + ((REGION_WIDTH / 2) - 20)];
				if (HeightMap[tile_idx] < 7) HeightMap[tile_idx] = 7;
			}
		}
	}
	return subregion_idx;
}

void add_dig_target(int X, int Y, int radius, int depth, TMap<int, region_water_feature_tile> &dig_targets, TArray<uint8_t> &pooled_water, TArray<uint8_t> &heightmap) noexcept {
	using namespace nfu;

	auto min_altitude = REGION_DEPTH;
	for (auto y = 0 - radius; y<radius; ++y) {
		for (auto x = 0 - radius; x < radius; ++x) {
			const auto actual_x = X + x;
			const auto actual_y = Y + y;
			const auto idx = (actual_y * REGION_WIDTH) + actual_x;
			if (idx > 0 && heightmap[idx] < min_altitude) min_altitude = heightmap[idx];
		}
	}

	for (auto y = 0 - radius; y<radius; ++y) {
		for (auto x = 0 - radius; x<radius; ++x) {
			const auto actual_x = X + x;
			const auto actual_y = Y + y;
			if (actual_x > -1 && actual_x < REGION_WIDTH && actual_y > -1 && actual_y < REGION_HEIGHT) {
				const auto idx = (actual_y * REGION_WIDTH) + actual_x;
				if (dig_targets.Find(idx) == nullptr) {
					const auto has_water = pooled_water[idx] > 0;
					if (idx > 0 && idx < REGION_TILES_COUNT) dig_targets[idx] = region_water_feature_tile{ idx, has_water, depth, heightmap[idx] };
				}
			}
		}
	}
}

void UNRegion::JustAddWater(TArray<uint8> &HeightMap, RandomNumberGenerator * rng, TArray<uint8> &PooledWater, TArray<TPair<int, uint8>> &WaterSpawners) {
	using namespace nfu;
	auto river_starts_here = false;
	auto river_terminates_here = false;
	auto has_river = false;

	TArray<int> river_entry;
	river_entry.AddZeroed(4);
	auto river_exit = 0;

	for (const auto &river : planet->Rivers) {
		if (river.start_x == RegionX && river.start_y == RegionY) {
			river_starts_here = true;
			has_river = true;
		}

		auto last_x = river.start_x;
		auto last_y = river.start_y;

		std::size_t i = 0;
		for (const auto &s : river.steps) {
			if (s.x == RegionX && s.y == RegionY) {
				has_river = true;

				if (last_x < s.x) { ++river_entry[0]; }
				if (last_x > s.x) { ++river_entry[1]; }
				if (last_y < s.y) { ++river_entry[2]; }
				if (last_y > s.y) { ++river_entry[3]; }

				if (i + 1 < river.steps.Num()) {
					const auto next_x = river.steps[i + 1].x;
					const auto next_y = river.steps[i + 1].y;

					if (next_x < s.x) { river_exit = 1; }
					if (next_x > s.x) { river_exit = 2; }
					if (next_y < s.y) { river_exit = 3; }
					if (next_y > s.y) { river_exit = 4; }
				}
				else {
					river_terminates_here = true;
				}
			}
			last_x = s.x;
			last_y = s.y;
			++i;
		}
	}

	if (!has_river) return;

	// Determine a confluence point - mid-point for the rivers
	auto mid_ok = false;
	int midpoint_x, midpoint_y;
	while (!mid_ok) {
		midpoint_x = rng->RollDice(1, REGION_WIDTH / 2) + REGION_WIDTH / 4;
		midpoint_y = rng->RollDice(1, REGION_HEIGHT / 2) + REGION_HEIGHT / 4;
		const auto d = distance2d(midpoint_x, midpoint_y, REGION_WIDTH / 2, REGION_HEIGHT / 2);
		if (d > 15.0f) mid_ok = true;
	}

	// Run rivers to the confluence
	TMap<int, region_water_feature_tile> dig_targets;

	const auto dig_river = [&HeightMap, &PooledWater, &dig_targets](int X, int Y) {
		add_dig_target(X, Y, 2, 2, dig_targets, PooledWater, HeightMap);
	};
	const auto dig_exit_river = [&HeightMap, &PooledWater, &dig_targets](int X, int Y) {
		add_dig_target(X, Y, 2, 2, dig_targets, PooledWater, HeightMap);
	};

	for (auto i = 0; i<river_entry[0]; ++i) {
		auto start_x = 0;
		auto start_y = rng->RollDice(1, REGION_HEIGHT / 2) + REGION_HEIGHT / 4 - 1;
		line_func(start_x, start_y, midpoint_x, midpoint_y, dig_river);
		WaterSpawners.Emplace(TPair<int, uint8>{ 1, (start_y * REGION_WIDTH) + start_x });
	}
	for (int i = 0; i<river_entry[1]; ++i) {
		int start_x = REGION_WIDTH - 1;
		int start_y = rng->RollDice(1, REGION_HEIGHT / 2) + REGION_HEIGHT / 4 - 1;
		line_func(start_x, start_y, midpoint_x, midpoint_y, dig_river);
		WaterSpawners.Emplace(TPair<int, uint8>{ 1, (start_y * REGION_WIDTH) + start_x });
	}
	for (int i = 0; i<river_entry[2]; ++i) {
		int start_x = rng->RollDice(1, REGION_WIDTH / 2) + REGION_WIDTH / 4 - 1;
		int start_y = 0;
		line_func(start_x, start_y, midpoint_x, midpoint_y, dig_river);
		WaterSpawners.Emplace(TPair<int, uint8>{ 1, (start_y * REGION_WIDTH) + start_x });
	}
	for (int i = 0; i<river_entry[3]; ++i) {
		int start_x = rng->RollDice(1, REGION_WIDTH / 2) + REGION_WIDTH / 4 - 1;
		int start_y = REGION_HEIGHT - 1;
		line_func(start_x, start_y, midpoint_x, midpoint_y, dig_river);
		WaterSpawners.Emplace(TPair<int, uint8>{ 1, (start_y * REGION_WIDTH) + start_x });
	}

	// Run confluence to the exit
	if (!river_terminates_here) {
		int end_x, end_y;
		if (river_exit == 1) { end_x = 0; end_y = rng->RollDice(1, REGION_HEIGHT / 2) + REGION_HEIGHT / 4; }
		if (river_exit == 2) { end_x = REGION_WIDTH; end_y = rng->RollDice(1, REGION_HEIGHT / 2) + REGION_HEIGHT / 4; }
		if (river_exit == 3) { end_x = rng->RollDice(1, REGION_WIDTH / 2) + REGION_WIDTH / 4; end_y = 0; }
		if (river_exit == 4) { end_x = rng->RollDice(1, REGION_WIDTH / 2) + REGION_WIDTH / 4; end_y = REGION_HEIGHT; }
		line_func(midpoint_x, midpoint_y, end_x, end_y, dig_exit_river);
		WaterSpawners.Emplace(TPair<int, uint8>{ 0, (end_y * REGION_WIDTH) + end_x });
	}

	// Actually dig out the rivers

	// 1 - Find the lowest point on the river network
	int min_altitude = REGION_DEPTH;
	for (const auto &t : dig_targets) {
		if (t.Value.altitude < min_altitude) min_altitude = t.Value.altitude;
	}

	// 2 - Dig down - the rivers are (lowest-point - depth)
	for (const auto &t : dig_targets) {
		if (!t.Value.has_water_already) {
			HeightMap[t.Value .idx] = min_altitude - 1;
			PooledWater[t.Value.idx] = 20;
		}
	}
}

/*
* Retrieve all of the materials that can make up different strata layers
*/
void get_strata_materials(NRaws * raws, TArray<size_t> &soils, TArray<size_t> &sedimentaries,	TArray<size_t> &igneouses, TArray<size_t> &sands) noexcept
{
	using namespace rawdefs;
	std::size_t i = 0;
	for (auto &it : raws->material_defs) {
		if (it.spawn_type == material_def_spawn_type_t::SOIL) soils.Emplace(i);
		if (it.spawn_type == material_def_spawn_type_t::SAND) sands.Emplace(i);
		if (it.spawn_type == material_def_spawn_type_t::ROCK && it.layer == "sedimentary") sedimentaries.Emplace(i);
		if (it.spawn_type == material_def_spawn_type_t::ROCK && it.layer == "igneous") igneouses.Emplace(i);
		++i;
	}
}

regiondefs::strata_t UNRegion::BuildStrata(TArray<uint8> &HeightMap, RandomNumberGenerator * rng) {
	using namespace nfu;
	using namespace regiondefs;

	UNoxGameInstance * game = Cast<UNoxGameInstance>(UGameplayStatics::GetGameInstance(this));
	auto raws = game->GetRaws();

	TPair<FNBiome, rawdefs::biome_type_t> biome;
	biome.Key = planet->Biomes[planet->Landblocks[planet->idx(RegionX, RegionY)].biome_idx];
	biome.Value = *raws->get_biome_def(biome.Key.type);

	strata_t result;
	result.strata_map.AddZeroed(REGION_TILES_COUNT);

	soils.Empty();
	sands.Empty();
	sedimintaries.Empty();
	igneouses.Empty();

	get_strata_materials(raws, soils, sedimintaries, igneouses, sands);

	// Locating strata
	const auto n_strata = (REGION_WIDTH + REGION_HEIGHT) * 4 + rng->RollDice(1, 64);
	result.strata_map.AddUninitialized(REGION_TILES_COUNT);
	result.material_idx.AddUninitialized(n_strata);
	for (int i = 0; i < REGION_TILES_COUNT; ++i) { result.strata_map[i] = 1; }
	for (int i = 0; i < n_strata; ++i) { result.material_idx[i] = 1; }
	result.counts.AddUninitialized(n_strata);
	for (int i = 0; i < n_strata; ++i) { result.counts[i] = TTuple<int, int, int, int>(0, 0, 0, 0); }

	FastNoise biome_noise(planet->PerlinSeed + (RegionY * REGION_WIDTH) + RegionX);
	biome_noise.SetNoiseType(FastNoise::Cellular);

	for (auto z = 0; z<REGION_DEPTH; ++z) {
		const auto Z = static_cast<float>(z)*8.0F;
		for (auto y = 0; y<REGION_WIDTH; ++y) {
			const auto Y = static_cast<float>(y)*8.0F;
			for (int x = 0; x<REGION_HEIGHT; ++x) {
				const auto X = static_cast<float>(x)*8.0F;
				const auto cell_noise = biome_noise.GetCellular(X, Y, Z);
				const auto biome_ramp = (cell_noise + 2.0F) / 4.0F;
				//assert(biome_ramp > 0.0F);
				//assert(biome_ramp < 1.0F);
				const auto biome_idx = static_cast<int>(biome_ramp * static_cast<float>(n_strata));
				++result.counts[biome_idx].Get<0>();
				result.counts[biome_idx].Get<1>() += x;
				result.counts[biome_idx].Get<2>() += y;
				result.counts[biome_idx].Get<3>() += z;

				const auto map_idx = mapidx(x, y, z);
				result.strata_map[map_idx] = biome_idx;
				//std::cout << x << "/" << y << "/" << z << " : " << X << "/" << Y << "/" << Z << " = " << cell_noise << "/" << biome_ramp << " : " << biome_idx << "\n";
			}
		}
	}

	int count_used = 0;
	for (int i = 0; i<n_strata; ++i) {
		if (result.counts[i].Get<0>() > 0) {
			++count_used;
			result.counts[i].Get<1>() /= result.counts[i].Get<0>();
			result.counts[i].Get<2>() /= result.counts[i].Get<0>();
			result.counts[i].Get<3>() /= result.counts[i].Get<0>();

			int &n = result.counts[i].Get<0>();
			int &x = result.counts[i].Get<1>();
			int &y = result.counts[i].Get<2>();
			int &z = result.counts[i].Get<3>();

			const uint8_t altitude_at_center = HeightMap[(y * REGION_WIDTH) + x] + REGION_DEPTH / 2;

			if (z>altitude_at_center - (1 + rng->RollDice(1, 4))) {
				// Soil
				const auto roll = rng->RollDice(1, 100);
				if (roll < biome.Value.soil_pct) {
					const auto soil_idx = rng->RollDice(1, soils.Num()) - 1;
					//std::cout << material_name(soils[soil_idx]) << "\n";
					result.material_idx[i] = soils[soil_idx];
				}
				else {
					const auto sand_idx = rng->RollDice(1, sands.Num()) - 1;
					//std::cout << material_name(sands[sand_idx]) << "\n";
					result.material_idx[i] = sands[sand_idx];
				}
			}
			else if (z>(altitude_at_center - 10) / 2) {
				// Sedimentary
				const int sed_idx = rng->RollDice(1, sedimintaries.Num()) - 1;
				//std::cout << material_name(sedimintaries[sed_idx]) << "\n";
				result.material_idx[i] = sedimintaries[sed_idx];
			}
			else {
				// Igneous
				const int ig_idx = rng->RollDice(1, igneouses.Num()) - 1;
				//std::cout << material_name(igneouses[ig_idx]) << "\n";
				result.material_idx[i] = igneouses[ig_idx];
			}
		}
		else {
			//std::cout << "Warning - we didn't think this strata was in use!\n";
			result.material_idx[i] = 1;
		}
	}
	//std::cout << count_used << " strata detected, " << n_strata - count_used << " unused.\n";

	return result;
}

void UNRegion::LayStrata(TArray<uint8> &HeightMap, RandomNumberGenerator * rng, TArray<uint8> &PooledWater, TArray<TPair<int, uint8>> &WaterSpawners, regiondefs::strata_t &strata) {
	using namespace nfu;
	using namespace regiondefs;

	UNoxGameInstance * game = Cast<UNoxGameInstance>(UGameplayStatics::GetGameInstance(this));
	auto raws = game->GetRaws();

	TPair<FNBiome, rawdefs::biome_type_t> biome;
	biome.Key = planet->Biomes[planet->Landblocks[planet->idx(RegionX, RegionY)].biome_idx];
	biome.Value = *raws->get_biome_def(biome.Key.type);

	// For vegetation
	int max_veg_probability = 0;
	for (const auto &vegprob : biome.Value.plants) max_veg_probability += vegprob.Value;

	// Lay down layers
	for (int y = 0; y<REGION_HEIGHT; ++y) {
		for (int x = 0; x<REGION_WIDTH; ++x) {
			const int cell_idx = (y * REGION_WIDTH) + x;
			uint8_t altitude = HeightMap[cell_idx];
			if (altitude > REGION_DEPTH - 10) altitude = REGION_DEPTH - 1;
			bool wet = false;
			if (altitude < 5) wet = true;

			// The bottom layer is always SMR to avoid spill-through
			SetTileType(mapidx(x, y, 0), tile_type::SEMI_MOLTEN_ROCK);

			int z = 1;
			while (z < altitude) {
				// Place Lava area - SMR for now
				if (x == 0 || x == REGION_WIDTH - 1 || y == 0 || y == REGION_HEIGHT - 1) {
					SetTileType(mapidx(x, y, z), tile_type::SEMI_MOLTEN_ROCK);
				}
				else {
					SetTileType(mapidx(x, y, z), tile_type::OPEN_SPACE);
					SetTileMaterial(mapidx(x, y, z), 0);
					// TODO: Just add magma
				}
				++z;
			}

			// Next up is rock, until the soil layer
			while (z < FMath::Min(altitude + 64, REGION_DEPTH - 20)) {
				// Place rock and soil
				SetTileType(mapidx(x, y, z), tile_type::SOLID);
				std::size_t material_idx;
				const int idx = mapidx(x, y, z);
				if (idx < strata.strata_map.Num()) {
					const int strata_idx = strata.strata_map[mapidx(x, y, z)];
					if (strata_idx < strata.material_idx.Num()) {
						material_idx = strata.material_idx[strata_idx];
					}
					else {
						material_idx = 1;
						//glog(log_target::GAME, log_severity::warning, "Warning - exceeded strata_material size");
					}
				}
				else {
					//glog(log_target::GAME, log_severity::warning, "Warning - exceeded strata_map size ({0})", strata.strata_map.size());
					material_idx = 1;
				}
				SetTileMaterial(mapidx(x, y, z), material_idx);
				++z;
			}

			// Populate the surface tile at z-1
			reveal(mapidx(x, y, z - 1));
			reveal(mapidx(x, y, z - 2));
			SetTileType(mapidx(x, y, z - 1), tile_type::FLOOR);
			if (wet) {
				SetWaterLevel(mapidx(x, y, z - 1), 10); // Below the water line; flood it!
			}
			else {
				if (PooledWater[cell_idx]>0) {
					int w = PooledWater[cell_idx];
					int Z = z - 1;

					for (const auto &ws : WaterSpawners) {
						if (ws.Key == cell_idx) {
							// TODO: Support entities!
							//bengine::create_entity()->assign(position_t{ x,y,Z })->assign(water_spawner_t{ ws.second });
						}
					}

					SetWaterLevel(mapidx(x, y, Z), 10);
					//while (w > 0) {
					//    set_water_level(mapidx(x,y,Z), 10);
					//    w -= 10;
					//    ++Z;
					//}
				}
				else {

					SetWaterLevel(mapidx(x, y, z - 1), 0);

					// Soil/sand
					const auto roll = rng->RollDice(1, 100);
					if (roll < biome.Value.soil_pct) {
						const int soil_idx = rng->RollDice(1, soils.Num()) - 1;
						//std::cout << material_name(soils[soil_idx]) << "\n";
						SetTileMaterial(mapidx(x, y, z - 1), soils[soil_idx]);
					}
					else {
						const int sand_idx = rng->RollDice(1, sands.Num()) - 1;
						//std::cout << material_name(sands[sand_idx]) << "\n";
						SetTileMaterial(mapidx(x, y, z - 1), sands[sand_idx]);
					}

					// Surface coverage
					FString veg_type;
					if (max_veg_probability > 0) {
						auto die_roll = rng->RollDice(1, max_veg_probability);
						for (const auto &veg : biome.Value.plants) {
							die_roll -= veg.Value;
							if (die_roll < 1) {
								veg_type = veg.Key;
								break;
							}
						}
					}
					if (veg_type.Len()==0) veg_type = "none";

					if (veg_type != "none") {
						auto finder = raws->get_plant_idx(veg_type);
						SetVegType(mapidx(x, y, z - 1), finder);
						SetVegHp(mapidx(x, y, z - 1), 10);
						SetVegTicker(mapidx(x, y, z - 1), 1);
						SetVegLifecycle(mapidx(x, y, z - 1), raws->get_plant_def(finder)->lifecycle[4] - 1);
					}
				}
			}

			while (z<REGION_DEPTH) {
				// Place sky
				reveal(mapidx(x, y, z));
				SetTileType(mapidx(x, y, z), tile_type::OPEN_SPACE);
				if (z < 69) SetWaterLevel(mapidx(x, y, z), 10); // Below the water line; flood it!
				++z;
			}
		}
	}
}

void UNRegion::BuildRamps(const TArray<uint8> &PooledWater) {
	using namespace nfu;
	using namespace regiondefs;

	for (int y = 1; y<REGION_HEIGHT - 1; ++y) {
		for (int x = 1; x<REGION_WIDTH - 1; ++x) {
			const int z = GroundZ(x, y);
			if (TileType[mapidx(x, y, z)] == tile_type::FLOOR && PooledWater[(y*REGION_WIDTH) + x] == 0) {
				bool is_ramp = false;
				if (TileType[mapidx(x, y - 1, z + 1)] == tile_type::FLOOR) is_ramp = true;
				if (TileType[mapidx(x, y + 1, z + 1)] == tile_type::FLOOR) is_ramp = true;
				if (TileType[mapidx(x - 1, y, z + 1)] == tile_type::FLOOR) is_ramp = true;
				if (TileType[mapidx(x + 1, y, z + 1)] == tile_type::FLOOR) is_ramp = true;

				if (is_ramp) {
					const auto idx = mapidx(x, y, z);
					SetTileType(idx, tile_type::RAMP);
					reveal(idx);
				}
			}
		}
	}

	for (int z = 1; z<REGION_DEPTH - 1; ++z) {
		for (int y = 1; y<REGION_HEIGHT - 1; ++y) {
			for (int x = 1; x<REGION_WIDTH - 1; ++x) {
				if (TileType[mapidx(x, y, z)] == tile_type::SOLID) {
					bool is_edge = false;
					if (TileType[mapidx(x, y - 1, z)]== tile_type::FLOOR  || TileType[mapidx(x, y - 1, z)] == tile_type::OPEN_SPACE || TileType[mapidx(x, y - 1, z)] == tile_type::RAMP) is_edge = true;
					if (TileType[mapidx(x, y + 1, z)] == tile_type::FLOOR || TileType[mapidx(x, y + 1, z)] == tile_type::OPEN_SPACE || TileType[mapidx(x, y + 1, z)] == tile_type::RAMP) is_edge = true;
					if (TileType[mapidx(x - 1, y, z)] == tile_type::FLOOR || TileType[mapidx(x - 1, y, z)] == tile_type::OPEN_SPACE || TileType[mapidx(x - 1, y, z)] == tile_type::RAMP) is_edge = true;
					if (TileType[mapidx(x + 1, y, z)] == tile_type::FLOOR || TileType[mapidx(x + 1, y, z)] == tile_type::OPEN_SPACE || TileType[mapidx(x + 1, y, z)] == tile_type::RAMP) is_edge = true;

					if (is_edge) {
						reveal(mapidx(x, y, z));
					}
				}
			}
		}
	}
}

void UNRegion::BuildBeaches() {
	using namespace nfu;
	using namespace regiondefs;

	UNoxGameInstance * game = Cast<UNoxGameInstance>(UGameplayStatics::GetGameInstance(this));
	auto raws = game->GetRaws();

	const auto sand = raws->get_material_by_tag("yellow_sand");
	for (int y = 1; y<WORLD_HEIGHT - 1; ++y) {
		for (int x = 1; x<WORLD_WIDTH; ++x) {
			const int z = GroundZ(x, y);
			if (TileType[mapidx(x, y, z)] == tile_type::FLOOR && WaterLevel[mapidx(x, y, z)] == 0) {
				bool is_beach = false;
				if (WaterLevel[mapidx(x, y - 1, z - 1)] > 0) is_beach = true;
				if (WaterLevel[mapidx(x, y + 1, z - 1)] > 0) is_beach = true;
				if (WaterLevel[mapidx(x - 1, y, z - 1)] > 0) is_beach = true;
				if (WaterLevel[mapidx(x + 1, y, z - 1)] > 0) is_beach = true;

				if (is_beach) {
					SetTileMaterial(mapidx(x, y, z), sand);
					SetVegType(mapidx(x, y, z), 0);
				}
			}
		}
	}
}

void UNRegion::BuildTrees(RandomNumberGenerator * rng, bool StartingArea) {
	using namespace nfu;
	using namespace regiondefs;

	UNoxGameInstance * game = Cast<UNoxGameInstance>(UGameplayStatics::GetGameInstance(this));
	auto raws = game->GetRaws();

	TPair<FNBiome, rawdefs::biome_type_t> biome;
	biome.Key = planet->Biomes[planet->Landblocks[planet->idx(RegionX, RegionY)].biome_idx];
	biome.Value = *raws->get_biome_def(biome.Key.type);

	const auto d_chance = biome.Value.deciduous_tree_chance;
	const auto e_chance = biome.Value.evergreen_tree_chance;

	for (auto y = 10; y<REGION_HEIGHT - 10; ++y) {
		for (auto x = 10; x<REGION_WIDTH - 10; ++x) {
			const auto z = GroundZ(x, y);
			const auto idx = mapidx(x, y, z);
			const auto crash_distance = StartingArea ? distance2d(x, y, REGION_WIDTH / 2, REGION_HEIGHT / 2) : 200;
			if (crash_distance > 20 && TileType[idx] == tile_type::FLOOR && WaterLevel[idx] == 0 && CanSeeSky(x, y, z)) {
				auto dice_roll = rng->RollDice(1, 1000);
				if (dice_roll <= d_chance) {
					// Grow a decidous tree
					TreeId[idx] = NextTreeID;
					TileType[idx] = tile_type::TREE_TRUNK;
					++NextTreeID;
				}
				else {
					dice_roll = rng->RollDice(1, 1000);
					if (dice_roll < e_chance) {
						TreeId[idx] = NextTreeID;
						TileType[idx] = tile_type::TREE_TRUNK;
						++NextTreeID;
					}
				}
			}
		}
	}
}

void UNRegion::BuildDebrisTrail(const int &crash_x, const int &crash_y) {
	using namespace nfu;
	using namespace regiondefs;

	for (auto x = crash_x - (REGION_WIDTH / 4); x<crash_x; ++x) {
		for (auto y = crash_y - 3; y<crash_y + 4; ++y) {
			auto z = GroundZ(x, y);
			const auto idx = mapidx(x, y, z);
			SetVegType(idx, 0);
			if (TreeId[idx] > 0) {
				// Tree needs destroying
				const auto target_tree = TreeId[idx];

				auto number_of_logs = 0;
				auto tree_idx = 0;
				auto lowest_z = 1000;

				for (auto Z = 0; Z < REGION_DEPTH; ++Z) {
					for (auto Y = 0; Y < REGION_HEIGHT; ++Y) {
						for (auto X = 0; X < REGION_WIDTH; ++X) {
							const auto tidx = mapidx(X, Y, Z);
							if (TreeId[tidx] == target_tree) {
								if (Z < lowest_z) {
									lowest_z = Z;
									tree_idx = tidx;
								}

								SetTileType(tidx, tile_type::OPEN_SPACE);
								TreeId[tidx] = 0;
								++number_of_logs;
							}
						}
					}
				}

				SetTileType(idx, tile_type::FLOOR);
				// Spawn wooden logs
				number_of_logs = (number_of_logs / 20) + 1;
				for (auto i = 0; i<number_of_logs; ++i) {
					// TODO: Support entities
					//const std::string cname = "Crashing Ship";
					//spawn_item_on_ground(x, y, z, "wood_log", get_material_by_tag("wood"), 3, 100, 0, cname);
				}
			}

		}
	}
}

void UNRegion::BuildGameComponents(const int &crash_x, const int &crash_y, const int &crash_z) {
	UNoxGameInstance * game = Cast<UNoxGameInstance>(UGameplayStatics::GetGameInstance(this));
	auto ecs = game->GetECS();

	calendar_t calendar;
	calendar.defined_shifts.Emplace(shift_t{ "Early Shift",{
		WORK_SHIFT, WORK_SHIFT, WORK_SHIFT, WORK_SHIFT, WORK_SHIFT, WORK_SHIFT, WORK_SHIFT, WORK_SHIFT, LEISURE_SHIFT, LEISURE_SHIFT, LEISURE_SHIFT, LEISURE_SHIFT, LEISURE_SHIFT, LEISURE_SHIFT, LEISURE_SHIFT, LEISURE_SHIFT, SLEEP_SHIFT, SLEEP_SHIFT, SLEEP_SHIFT, SLEEP_SHIFT, SLEEP_SHIFT, SLEEP_SHIFT, SLEEP_SHIFT, SLEEP_SHIFT }
		});
	calendar.defined_shifts.Emplace(shift_t{ "Day Shift",{
		SLEEP_SHIFT, SLEEP_SHIFT, SLEEP_SHIFT, SLEEP_SHIFT, SLEEP_SHIFT, SLEEP_SHIFT, SLEEP_SHIFT, SLEEP_SHIFT,	WORK_SHIFT, WORK_SHIFT, WORK_SHIFT, WORK_SHIFT, WORK_SHIFT, WORK_SHIFT, WORK_SHIFT, WORK_SHIFT, LEISURE_SHIFT, LEISURE_SHIFT, LEISURE_SHIFT, LEISURE_SHIFT, LEISURE_SHIFT, LEISURE_SHIFT, LEISURE_SHIFT, LEISURE_SHIFT }
		});
	calendar.defined_shifts.Emplace(shift_t{ "Late Shift",{
		LEISURE_SHIFT, LEISURE_SHIFT, LEISURE_SHIFT, LEISURE_SHIFT, LEISURE_SHIFT, LEISURE_SHIFT, LEISURE_SHIFT, LEISURE_SHIFT, SLEEP_SHIFT, SLEEP_SHIFT, SLEEP_SHIFT, SLEEP_SHIFT, SLEEP_SHIFT, SLEEP_SHIFT, SLEEP_SHIFT, SLEEP_SHIFT,	WORK_SHIFT, WORK_SHIFT, WORK_SHIFT, WORK_SHIFT, WORK_SHIFT, WORK_SHIFT, WORK_SHIFT, WORK_SHIFT }
		});

	auto game_id = ecs->ecs.AddEntity();
	ecs->ecs.Assign(game_id, world_position_t{ RegionX, RegionY, crash_x, crash_y, crash_z });
	ecs->ecs.Assign(game_id, std::move(calendar));
	ecs->ecs.Assign(game_id, designations_t{});
	ecs->ecs.Assign(game_id, logger_t{});
	ecs->ecs.Assign(game_id, camera_options_t{ TOP_DOWN, false, 12 });
	ecs->ecs.Assign(game_id, mining_designations_t{});
	ecs->ecs.Assign(game_id, farming_designations_t{});
	ecs->ecs.Assign(game_id, building_designations_t{});
	ecs->ecs.Assign(game_id, architecture_designations_t{});
}

void UNRegion::BuildEscapePod(const int &crash_x, const int &crash_y, const int &crash_z) {
	int z = FMath::Max(crash_z - 2, 1);
	FString ThePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() + FString("rex/spaceship.xp"));
	char * path_c_str = TCHAR_TO_ANSI(*ThePath);
	RexReader rr;
	rr.LoadFile(path_c_str);

	rr.GetLayerCount();
	auto ship = rr.GetTileMap();

	for (int layer = 0; layer<5; ++layer) {
		for (int Y = 0; Y<rr.GetLayerHeight(); ++Y) {
			for (int X = 0; X<rr.GetLayerWidth(); ++X) {
				const int x = X - 5 + crash_x;
				const int y = Y - 11 + crash_y;

				const auto &output = ship->Layers[layer].Tiles[(Y * rr.GetLayerWidth()) + X];
				const auto &glyph = output.CharacterCode;
				if (glyph != 32) reveal(mapidx(x, y, z));
				if (!( output.BackgroundRed == 255 && output.BackgroundGreen == 0 && output.BackgroundBlue == 255 )) {
					if (glyph == 219) {
						add_construction(x, y, z, "ship_wall", true, 0);
					}
					else if (glyph == 'W') {
						add_construction(x, y, z, "ship_window", true, 0);
					}
					else if (glyph == 176) {
						add_construction(x, y, z, "ship_floor", false, 0);
					}
					else if (glyph == 'X') {
						add_construction(x, y, z, "ship_updown", false, 0);
					}
					else if (glyph == '<') {
						add_construction(x, y, z, "ship_up", false, 0);
					}
					else if (glyph == '>') {
						add_construction(x, y, z, "ship_down", false, 0);
					}
					else if (glyph == 178) {
						add_construction(x, y, z, "solar_panel", false, 0);
					}
					else if (glyph == 241) {
						add_construction(x, y, z, "battery", false, 0);
					}
					else if (glyph == 'X') {
						add_construction(x, y, z, "ship_updown", false, 0);
					}
					else if (glyph == '0') {
						add_construction(x, y, z, "cryo_bed", false, 0);
					}
					else if (glyph == 'X') {
						add_construction(x, y, z, "ship_updown", false, 0);
					}
					else if (glyph == 236) {
						add_construction(x, y, z, "storage_locker", false, 0);
					}
					else if (glyph == 'C') {
						add_construction(x, y, z, "cordex", false, 0);
					}
					else if (glyph == 243) {
						add_construction(x, y, z, "ship_defense_turret", false, 0);
					}
					else if (glyph == 251) {
						add_construction(x, y, z, "small_replicator", false, 0);
					}
					else if (glyph == 232) {
						add_construction(x, y, z, "rtg", false, 0);
					}
					else if (glyph == 197) {
						add_construction(x, y, z, "ship_door", false, 0);
					}
					else if (glyph == 'L') {
						add_construction(x, y, z, "ship_lamp", false, 0);
					}
					else {
						//if (output->glyph != 32)
						//glog(log_target::LOADER, log_severity::warning, "No handler for {0} ({1})", (char)output->glyph, output->glyph);
					}
				}
			}
		}
		++z;
		//std::cout << z << "\n";
	}
}

void UNRegion::add_construction(const int x, const int y, const int z, const FString type, bool solid, const size_t &civ_owner) noexcept {
	UNoxGameInstance * game = Cast<UNoxGameInstance>(UGameplayStatics::GetGameInstance(this));
	auto raws = game->GetRaws();

	const auto idx = mapidx(x, y, z);
	const auto plasteel = raws->get_material_by_tag("plasteel");
	const auto wood = raws->get_material_by_tag("wood");
	using namespace regiondefs;

	set_tile(idx, tile_type::FLOOR, false, false, plasteel, 0, true, true);

	if (type == "ship_wall") {
		set_tile(idx, tile_type::WALL, true, true, plasteel, 0, true, true);
	}
	else if (type == "ship_window") {
		set_tile(idx, tile_type::WINDOW, true, false, plasteel, 0, true, true);
	}
	else if (type == "ship_floor") {
		set_tile(idx, tile_type::FLOOR, false, false, plasteel, 0, true, true);
	}
	else if (type == "hut_wall") {
		set_tile(idx, tile_type::WALL, true, true, wood, 0, true, true);
	}
	else if (type == "hut_floor") {
		set_tile(idx, tile_type::FLOOR, false, false, wood, 0, true, true);
	}
	else if (type == "ship_up") {
		set_tile(idx, tile_type::STAIRS_UP, false, false, plasteel, 0, true, true);
	}
	else if (type == "ship_down") {
		set_tile(idx, tile_type::STAIRS_DOWN, false, false, plasteel, 0, true, true);
	}
	else if (type == "ship_updown") {
		set_tile(idx, tile_type::STAIRS_UPDOWN, false, false, plasteel, 0, true, true);
	}
	else if (type == "hut_upstairs") {
		set_tile(idx, tile_type::STAIRS_UP, false, false, wood, 0, true, true);
	}
	else if (type == "hut_downstairs") {
		set_tile(idx, tile_type::STAIRS_DOWN, false, false, wood, 0, true, true);
	}
	else if (type == "hut_updownstairs") {
		set_tile(idx, tile_type::STAIRS_UPDOWN, false, false, wood, 0, true, true);
	}
	else if (type == "cordex") {
		add_building("cordex", x, y, z, civ_owner);
	}
	else if (type == "ship_defense_turret") {
		add_building("ship_defense_turret", x, y, z, civ_owner);
	}
	else if (type == "solar_panel") {
		add_building("solar_panel", x, y, z, civ_owner);
	}
	else if (type == "cryo_bed") {
		add_building("cryo_bed", x, y, z, civ_owner);
	}
	else if (type == "storage_locker") {
		add_building("storage_locker", x, y, z, civ_owner);
	}
	else if (type == "battery") {
		add_building("battery", x, y, z, civ_owner);
	}
	else if (type == "rtg") {
		add_building("rtg", x, y, z, civ_owner);
	}
	else if (type == "small_replicator") {
		add_building("small_replicator", x, y, z, civ_owner);
	}
	else if (type == "campfire") {
		add_building("camp_fire", x, y, z, civ_owner);
	}
	else if (type == "ship_door") {
		add_building("energy_door", x, y, z, civ_owner);
	}
	else if (type == "ship_lamp") {
		add_building("lamp", x, y, z, civ_owner);
	}
	else if (type == "door") {
		//std::cout << "Door owner: " << civ_owner << "\n";
		add_building("door", x, y, z, civ_owner);
	}
	else {
		//glog(log_target::LOADER, log_severity::error, "Don't know how to build a {0}", type);
	}
}

void UNRegion::add_building(FString tag, const int x, const int y, const int z, const size_t &civ_owner) noexcept {
	UNoxGameInstance * game = Cast<UNoxGameInstance>(UGameplayStatics::GetGameInstance(this));
	auto raws = game->GetRaws();
	auto ecs = game->GetECS();

	auto building = raws->get_building_def(tag);
	if (building == nullptr) {
		//glog(log_target::LOADER, log_severity::error, "Warning: do not know how to build: {0}", tag);
	}

	int bx = x;
	int by = y;
	if (building->width == 3 && building->height == 3)
	{
		--bx;
		--by;
	}

	const auto new_building_id = ecs->ecs.AddEntity();
	ecs->ecs.Assign(new_building_id, position_t{ bx, by, z });
	ecs->ecs.Assign(new_building_id, building_t{ tag, building->width, building->height, true, civ_owner, 10, 10, building->vox_model });
	ecs->ecs.Assign(new_building_id, name_t{ building->name, "" });

	for (const auto &provides : building->provides) {
		if (provides.provides == rawdefs::provides_sleep) ecs->ecs.Assign(new_building_id, construct_provides_sleep_t{});
		if (provides.provides == rawdefs::provides_storage) ecs->ecs.Assign(new_building_id, construct_container_t{});
	}

	const int offX = building->width == 3 ? -1 : 0;
	const int offY = building->height == 3 ? -1 : 0;
	for (int Y = y + offY; Y < y + building->height + offY; ++Y) {
		for (int X = x + offX; X < x + building->width + offX; ++X) {
			const int idx = mapidx(X, Y, z);
			BuildingId[idx] = new_building_id;
		}
	}

	if (tag == "storage_locker") {
		spawn_item_in_container(new_building_id, "personal_survival_shelter_kit", raws->get_material_by_tag("plasteel"), 6, 100, 0, "Eden Acme Corp");
		spawn_item_in_container(new_building_id, "personal_survival_shelter_kit", raws->get_material_by_tag("plasteel"), 6, 100, 0, "Eden Acme Corp");
		spawn_item_in_container(new_building_id, "personal_survival_shelter_kit", raws->get_material_by_tag("plasteel"), 6, 100, 0, "Eden Acme Corp");
		spawn_item_in_container(new_building_id, "camp_fire_kit", raws->get_material_by_tag("plasteel"), 6, 100, 0, "Eden Acme Corp");
		spawn_item_in_container(new_building_id, "fire_axe", raws->get_material_by_tag("plasteel"), 6, 100, 0, "Eden Acme Corp");
		spawn_item_in_container(new_building_id, "pickaxe", raws->get_material_by_tag("plasteel"), 6, 100, 0, "Eden Acme Corp");
		spawn_item_in_container(new_building_id, "hoe", raws->get_material_by_tag("plasteel"), 6, 100, 0, "Eden Acme Corp");
	}
	else if (tag == "cordex") {
		ecs->ecs.Assign(new_building_id, viewshed_t{ 16, false });
	}
	else if (tag == "lamp") {
		ecs->ecs.Assign(new_building_id, lightsource_t{ 16, FNColor{ 1.0f, 1.0f, 1.0f }, true });
	}
	else if (tag == "battery") {
		ecs->ecs.Assign(new_building_id, construct_power_t{ 20,0,0 });
	}
	else if (tag == "rtg") {
		ecs->ecs.Assign(new_building_id, construct_power_t{ 0,1,0 });
	}
	else if (tag == "solar_panel") {
		ecs->ecs.Assign(new_building_id, construct_power_t{ 00,0,1 });
	}
	else if (tag == "camp_fire") {
		ecs->ecs.Assign(new_building_id, lightsource_t{ 5, FNColor{ 1.0f, 1.0f, 0.0f } });
		ecs->ecs.Assign(new_building_id, smoke_emitter_t{});
	}
	else if (tag == "energy_door" || tag == "door") {
		ecs->ecs.Assign(new_building_id, construct_door_t{});
		ecs->ecs.Assign(new_building_id, receives_signal_t{});
	}
	else if (tag == "ship_defense_turret") {
		//std::cout << "Turret created\n";
		ecs->ecs.Assign(new_building_id, viewshed_t{ 8, false });
		ecs->ecs.Assign(new_building_id, turret_t{ 8, 2, 3, 8, 3, civ_owner });
		ecs->ecs.Assign(new_building_id, initiative_t{});
	}
}

void UNRegion::CreateSettler(const int x, const int y, const int z, RandomNumberGenerator * rng, int shift_id) noexcept {
	UNoxGameInstance * game = Cast<UNoxGameInstance>(UGameplayStatics::GetGameInstance(this));
	auto raws = game->GetRaws();
	auto ecs = game->GetECS();

	using namespace rawdefs;

	species_t species;
	game_stats_t stats;
	const auto species_def = raws->get_species_def("human");

	species.tag = "human";

	// Gender
	const auto gender_roll = rng->RollDice(1, 20);
	if (gender_roll < 11) {
		species.gender = MALE;
	}
	else {
		species.gender = FEMALE;
	}
	// Sexuality
	const auto sex_roll = rng->RollDice(1, 10);
	if (sex_roll < 9) {
		species.sexuality = HETEROSEXUAL;
	}
	else {
		species.sexuality = HOMOSEXUAL;
	}

	// Height/Weight
	if (species.gender == MALE)
	{
		species.height_cm = 147.0F + static_cast<float>(rng->RollDice(2, 10) * 2.5F);
		species.weight_kg = 54.0F + static_cast<float>(rng->RollDice(2, 8) * 0.45);
	}
	else
	{
		species.height_cm = 134.0F + (rng->RollDice(2, 10) * 2.5F);
		species.weight_kg = 38.0F + (rng->RollDice(2, 4) * 0.45);
	}

	// Hair/etc. this should be made more realistic one day!
	species.base_male_glyph = species_def->base_male_glyph;
	species.base_female_glyph = species_def->base_female_glyph;
	species.skin_color = species_def->skin_colors[rng->RollDice(1, species_def->skin_colors.Num()) - 1];

	species.bearded = false;
	if (species.gender == MALE)
	{
		const auto beard_roll = rng->RollDice(1, 20);
		if (beard_roll < 7)
		{
			species.bearded = true;
		}
		else
		{
			species.bearded = false;
		}
	}

	species.hair_color = species_def->hair_colors[rng->RollDice(1, species_def->hair_colors.Num()) - 1];

	species.hair_style = BALD;
	if (species.gender == MALE)
	{
		const auto style_roll = rng->RollDice(1, 5);
		switch (style_roll)
		{
		case 1:
			species.hair_style = BALD;
			break;
		case 2:
			species.hair_style = BALDING;
			break;
		case 3:
			species.hair_style = MOHAWK;
			break;
		case 4:
			species.hair_style = SHORT_HAIR;
			break;
		case 5:
			species.hair_style = LONG_HAIR;
			break;
		}
	}
	else
	{
		const auto style_roll = rng->RollDice(1, 4);
		switch (style_roll)
		{
		case 1:
			species.hair_style = SHORT_HAIR;
			break;
		case 2:
			species.hair_style = LONG_HAIR;
			break;
		case 3:
			species.hair_style = PIGTAILS;
			break;
		case 4:
			species.hair_style = TRIANGLE;
			break;
		}
	}

	// Name
	using namespace string_tables;
	FString first_name;
	if (species.gender == FEMALE) {
		first_name = to_proper_noun_case(raws->string_tables[FIRST_NAMES_FEMALE].random_entry(*rng));
	}
	else
	{
		first_name = to_proper_noun_case(raws->string_tables[FIRST_NAMES_MALE].random_entry(*rng));
	}

	const auto last_name = to_proper_noun_case(raws->string_tables[LAST_NAMES].random_entry(*rng));

	// Profession
	const auto starting_profession = raws->get_random_profession(*rng);
	stats.profession_tag = starting_profession->name;
	stats.original_profession = starting_profession->name;

	// Stats
	stats.strength = rng->RollDice(3, 6) + starting_profession->strength;
	stats.dexterity = rng->RollDice(3, 6) + starting_profession->dexterity;
	stats.constitution = rng->RollDice(3, 6) + starting_profession->constitution;
	stats.intelligence = rng->RollDice(3, 6) + starting_profession->intelligence;
	stats.wisdom = rng->RollDice(3, 6) + starting_profession->wisdom;
	stats.charisma = rng->RollDice(3, 6) + starting_profession->charisma;
	stats.comeliness = rng->RollDice(3, 6) + starting_profession->comeliness;
	stats.ethics = rng->RollDice(3, 6) + starting_profession->ethics;
	stats.age = 15 + rng->RollDice(3, 6);

	auto settler = ecs->ecs.AddEntity();

	// Life events
	auto year = 2525 - stats.age;
	auto age = 0;
	TArray<FString> event_buffer;

	while (year < 2522) {
		auto candidates = get_event_candidates(age, event_buffer);
		if (!candidates.Num() == 0) {
			const std::size_t idx = rng->RollDice(1, candidates.Num()) - 1;
			const auto event_name = candidates[idx];
			event_buffer.Emplace(event_name);
			auto ledef = raws->get_life_event(event_name);

			auto has_effect = false;
			if (ledef->strength != 0) has_effect = true;
			if (ledef->dexterity != 0) has_effect = true;
			if (ledef->constitution != 0) has_effect = true;
			if (ledef->intelligence != 0) has_effect = true;
			if (ledef->wisdom != 0) has_effect = true;
			if (ledef->charisma != 0) has_effect = true;
			if (ledef->comeliness != 0) has_effect = true;
			if (ledef->ethics != 0) has_effect = true;
			if (!ledef->skills.Num() == 0) has_effect = true;

			if (age == 0 || has_effect) {
				/*const auto finder = planet.history.settler_life_events.find(settler->id);
				const life_event_t event { year, event_name };
				if (finder == planet.history.settler_life_events.end()) {
					planet.history.settler_life_events[settler->id] = std::vector<life_event_t>{ event };
				}
				else {
					planet.history.settler_life_events[settler->id].push_back(event);
				}*/
				if (rng->RollDice(1, 10) > 7) {
					stats.strength += ledef->strength;
					stats.dexterity += ledef->dexterity;
					stats.constitution += ledef->constitution;
					stats.intelligence += ledef->intelligence;
					stats.wisdom += ledef->wisdom;
					stats.charisma += ledef->charisma;
					stats.comeliness += ledef->comeliness;
					stats.ethics += ledef->ethics;
				}
				for (const auto &skill : ledef->skills) {
					if (rng->RollDice(1, 10) > 7) {
						const auto skillfinder = stats.skills.Find(skill);
						if (skillfinder == nullptr) {
							stats.skills.Add(skill, skill_t{ 1, 0 });
						}
						else {
							if (stats.skills[skill].skill_level < 3) {
								++stats.skills[skill].skill_level;
								//glog(log_target::GAME, log_severity::info, "Raised skill in {0} to {1}", skill, +stats.skills[skill].skill_level);
							}
						}
					}
				}
			}
		}

		++year;
		++age;
	}
	//planet.history.settler_life_events[settler->id].push_back(life_event_t{ 2524, "ark_b" });
	//planet.history.settler_life_events[settler->id].push_back(life_event_t{ 2525, "planetfall" });

	auto base_hp = rng->RollDice(1, 10) + stat_modifier(stats.constitution);
	if (base_hp < 1) base_hp = 1;
	auto health = create_health_component_settler("human", base_hp, raws);

	settler_ai_t ai;
	ai.shift_id = shift_id;

	const auto angle = 0;
	ecs->ecs.Assign(settler, position_t{ x,y,z,angle });
	ecs->ecs.Assign(settler, renderable_composite_t{ RENDER_SETTLER, 2 });
	ecs->ecs.Assign(settler, std::move(species));
	ecs->ecs.Assign(settler, std::move(health));
	ecs->ecs.Assign(settler, std::move(stats));
	ecs->ecs.Assign(settler, std::move(ai));
	ecs->ecs.Assign(settler, name_t{ first_name, last_name });
	ecs->ecs.Assign(settler, viewshed_t{ 8, false });
	ecs->ecs.Assign(settler, initiative_t{});
	ecs->ecs.Assign(settler, ai_settler_new_arrival_t{});
	ecs->ecs.Assign(settler, sleep_clock_t{});
	ecs->ecs.Assign(settler, hunger_t{});
	ecs->ecs.Assign(settler, thirst_t{});

	// Create clothing items
	//std::cout << settler->id << "\n";
	for (auto item : starting_profession->starting_clothes) {
		if (item.Get<0>() == 0 || (item.Get<0>() == 1 && species.gender == MALE) || (item.Get<0>() == 2 && species.gender == FEMALE)) {
			const auto item_name = item.Get<2>();
			const auto slot_name = item.Get<1>();
			auto position = INVENTORY;
			if (slot_name == "head") position = HEAD;
			if (slot_name == "torso") position = TORSO;
			if (slot_name == "legs") position = LEGS;
			if (slot_name == "shoes") position = FEET;
			spawn_item_carried(settler, item_name, raws->get_material_by_tag("cloth"), position, ecs_item_quality::GREAT, 100, 0, "Eden Acme Corp", rng);
		}
	}

	// Add a raygun and energey cells
	const auto plasteel = raws->get_material_by_tag("plasteel");
	spawn_item_carried(settler, "ray_pistol", plasteel, EQUIP_RANGED, ecs_item_quality::GREAT, 100, 0, "Eden Acme Corp", rng);
	spawn_item_carried(settler, "small_energy_cell", plasteel, EQUIP_AMMO, ecs_item_quality::GREAT, 100, 0, "Eden Acme Corp", rng);
}

void UNRegion::decorate_item_categories(int &item, TBitArray<> &categories) noexcept
{
	UNoxGameInstance * game = Cast<UNoxGameInstance>(UGameplayStatics::GetGameInstance(this));
	auto raws = game->GetRaws();
	auto ecs = game->GetECS();

	using namespace rawdefs;

	if (categories[TOOL_CHOPPING]) ecs->ecs.Assign(item, item_chopping_t{});
	if (categories[TOOL_DIGGING]) ecs->ecs.Assign(item, item_digging_t{});
	if (categories[WEAPON_MELEE]) ecs->ecs.Assign(item, item_melee_t{});
	if (categories[WEAPON_RANGED]) ecs->ecs.Assign(item, item_ranged_t{});
	if (categories[WEAPON_AMMO]) ecs->ecs.Assign(item, item_ammo_t{});
	if (categories[ITEM_FOOD]) ecs->ecs.Assign(item, item_food_t{});
	if (categories[ITEM_SPICE]) ecs->ecs.Assign(item, item_spice_t{});
	if (categories[ITEM_DRINK]) ecs->ecs.Assign(item, item_drink_t{});
	if (categories[ITEM_HIDE]) ecs->ecs.Assign(item, item_hide_t{});
	if (categories[ITEM_BONE]) ecs->ecs.Assign(item, item_bone_t{});
	if (categories[ITEM_SKULL]) ecs->ecs.Assign(item, item_skull_t{});
	if (categories[ITEM_LEATHER]) ecs->ecs.Assign(item, item_leather_t{});
	if (categories[ITEM_FARMING]) ecs->ecs.Assign(item, item_farming_t{});
	if (categories[ITEM_SEED]) ecs->ecs.Assign(item, item_seed_t{});
	if (categories[ITEM_TOPSOIL]) ecs->ecs.Assign(item, item_topsoil_t{});
	if (categories[ITEM_FERTILIZER]) ecs->ecs.Assign(item, item_fertilizer_t{});
	if (categories[ITEM_FOOD_PREPARED]) ecs->ecs.Assign(item, item_food_prepared_t{});
}

void UNRegion::spawn_item_in_container(const int container_id, const FString &tag, const std::size_t &material, uint8_t quality, uint8_t wear, int creator_id, FString creator_name) noexcept {
	UNoxGameInstance * game = Cast<UNoxGameInstance>(UGameplayStatics::GetGameInstance(this));
	auto raws = game->GetRaws();
	auto ecs = game->GetECS();

	auto finder = raws->get_item_def(tag);
	if (finder == nullptr) {
		//glog(log_target::GAME, log_severity::warning, "Unknown item tag {0}", tag);
	}

	const auto mat = raws->get_material(material);

	//std::cout << "Spawning [" << tag << "], glyph " << +finder->glyph << "\n";

	auto entity = ecs->ecs.AddEntity();
	ecs->ecs.Assign(entity, item_stored_t{ container_id });
	ecs->ecs.Assign(entity, renderable_t{ finder->glyph, finder->glyph_ascii, finder->voxel_model });
	ecs->ecs.Assign(entity, item_t( tag, finder->name, material, finder->stack_size, finder->clothing_layer, raws ));
	ecs->ecs.Assign(entity, item_quality_t{ quality });
	ecs->ecs.Assign(entity, item_wear_t{ wear });
	ecs->ecs.Assign(entity, item_creator_t{ creator_id, creator_name });

	decorate_item_categories(entity, finder->categories);
}

void UNRegion::spawn_item_carried(const int holder_id, const FString &tag, const std::size_t &material, const ecs_item_location_t &loc, uint8_t quality, uint8_t wear, int creator_id, FString creator_name, RandomNumberGenerator * rng) noexcept
{
	UNoxGameInstance * game = Cast<UNoxGameInstance>(UGameplayStatics::GetGameInstance(this));
	auto raws = game->GetRaws();
	auto ecs = game->GetECS();

	const auto mat = raws->get_material(material);
	const auto clothing_finder = raws->get_clothing_by_tag(tag);
	if (clothing_finder) {
		// Clothing needs to be handled differently
		auto entity = ecs->ecs.AddEntity();
		ecs->ecs.Assign(entity, item_carried_t{ loc, holder_id });
		ecs->ecs.Assign(entity, renderable_t{ clothing_finder->clothing_glyph, clothing_finder->clothing_glyph, clothing_finder->voxel_model });
		ecs->ecs.Assign(entity, item_t{ tag, raws, rng });
		ecs->ecs.Assign(entity, item_quality_t{ quality });
		ecs->ecs.Assign(entity, item_wear_t{ wear });
		ecs->ecs.Assign(entity, item_creator_t{ creator_id, creator_name });
		//entity->component<item_t>()->material = material;
		// TODO: Implement entity->component interface
	}
	else {
		auto finder = raws->get_item_def(tag);
		if (finder == nullptr) {
			//glog(log_target::GAME, log_severity::warning, "Unknown item tag {0}", tag);
		}

		auto entity = ecs->ecs.AddEntity();
		ecs->ecs.Assign(entity, item_carried_t{ loc, holder_id });
		ecs->ecs.Assign(entity, renderable_t{ finder->glyph, finder->glyph_ascii, finder->voxel_model });
		ecs->ecs.Assign(entity, item_t{ tag, finder->name, material, finder->stack_size, finder->clothing_layer, raws });
		ecs->ecs.Assign(entity, item_quality_t{ quality });
		ecs->ecs.Assign(entity, item_wear_t{ wear });
		ecs->ecs.Assign(entity, item_creator_t{ creator_id, creator_name });
		decorate_item_categories(entity, finder->categories);
	}
}

TArray<FString> UNRegion::get_event_candidates(const int &age, const TArray<FString> &past) noexcept {
	using namespace rawdefs;

	UNoxGameInstance * game = Cast<UNoxGameInstance>(UGameplayStatics::GetGameInstance(this));
	auto raws = game->GetRaws();
	auto ecs = game->GetECS();

	TArray<FString> result;

	TSet<FString> unavailable;
	for (const auto &le : past) {
		auto lefinder = raws->get_life_event(le);
		for (const auto &no : lefinder->precludes_event) {
			unavailable.Add(no);
		}
	}

	raws->each_life_event([&unavailable, &result, &age, &past](FString tag, life_event_template * it) {
		if (age >= it->min_age && age <= it->max_age) {
			auto available = true;

			auto nope_check = unavailable.Find(tag);
			if (nope_check != nullptr) {
				available = false;
			}

			if (available && !it->requires_event.Num()==0) {
				available = false;
				for (const auto &req : it->requires_event) {
					for (const auto &p : past) {
						if (p == req) available = true;
					}
				}
			}

			if (available) {
				for (auto i = 0; i<it->weight; ++i) {
					result.Emplace(tag);
				}
			}

		}
	});

	return result;
}

/* CHUNKING */

void UNRegion::InitializeChunks() {
	using namespace nfu;

	DirtyChunks.Empty();
	for (int i=0; i<CHUNKS_TOTAL; ++i) DirtyChunks.Add(false);
	Chunks.Empty();
	Chunks.AddDefaulted(CHUNKS_TOTAL);

	for (int z = 0; z<CHUNK_DEPTH; ++z) {
		for (int y = 0; y<CHUNK_HEIGHT; ++y) {
			for (int x = 0; x<CHUNK_WIDTH; ++x) {
				const int idx = chunk_idx(x, y, z);
				SetupChunk(idx, x, y, z);
				MarkChunkDirty(idx);
			}
		}
	}
}

void UNRegion::UpdateChunks() {
	for (auto i = 0; i < nfu::CHUNKS_TOTAL; ++i) {
		if (DirtyChunks[i]) {
			UpdateChunk(i);
			DirtyChunks[i] = false;
		}
	}
}

void UNRegion::SetupChunk(const int &idx, const int &x, const int &y, const int &z) {
	using namespace nfu;
	Chunks[idx].base_x = x * CHUNK_SIZE;
	Chunks[idx].base_y = y * CHUNK_SIZE;
	Chunks[idx].base_z = z * CHUNK_SIZE;
	Chunks[idx].index = idx;
	Chunks[idx].layers.Empty();
	Chunks[idx].layers.AddDefaulted(CHUNK_SIZE);
}

static bool is_cube(const uint8_t type) {
	using namespace regiondefs;
	switch (type) {
	case tile_type::SOLID: return true;
	case tile_type::WALL: return true;
	case tile_type::SEMI_MOLTEN_ROCK: return true;
	default: return false;
	}
}

void UNRegion::greedy_floors(TMap<int, unsigned int> &floors, const int &chunk_idx, const int &chunk_z) {
	using namespace nfu;

	auto n_floors = floors.Num();
	const int base_x = Chunks[chunk_idx].base_x;
	const int base_y = Chunks[chunk_idx].base_y;
	const int base_z = Chunks[chunk_idx].base_z;

	while (!floors.Num()==0) {
		const auto first_floor = floors.CreateIterator();
		const auto base_region_idx = first_floor->Key;
		const auto texture_id = first_floor->Value;

		const auto tt = idxmap(base_region_idx);
		const auto tile_x = tt.Get<0>();
		const auto tile_y = tt.Get<1>();
		const auto tile_z = tt.Get<2>();
		auto width = 1;
		auto height = 1;

		floors.Remove(base_region_idx);
		auto idx_grow_right = base_region_idx + 1;
		auto x_coordinate = tile_x;
		auto right_finder = floors.Find(idx_grow_right);
		while (x_coordinate < REGION_WIDTH - 1 && idx_grow_right < mapidx(FMath::Min(REGION_WIDTH - 1, base_x + CHUNK_SIZE), tile_y, tile_z) && right_finder != nullptr && *right_finder == texture_id) {
			floors.Remove(idx_grow_right);
			++width;
			++idx_grow_right;
			++x_coordinate;
			right_finder = floors.Find(idx_grow_right);
		}

		if (tile_y < REGION_HEIGHT - 1) {
			auto y_progress = tile_y + 1;

			auto possible = true;
			while (possible && y_progress < base_y + CHUNK_SIZE && y_progress < REGION_HEIGHT - 1) {
				for (auto gx = tile_x; gx < tile_x + width; ++gx) {
					const auto candidate_idx = mapidx(gx, y_progress, tile_z);
					const auto vfinder = floors.Find(candidate_idx);
					if (vfinder == nullptr || *vfinder != texture_id) possible = false;
				}
				if (possible) {
					++height;
					for (auto gx = tile_x; gx < tile_x + width; ++gx) {
						const auto candidate_idx = mapidx(gx, y_progress, tile_z);
						floors.Remove(candidate_idx);
					}
				}

				++y_progress;
			}
		}

		Chunks[chunk_idx].layers[chunk_z].floors.Emplace(floor_t{ tile_x, tile_y, tile_z, width, height, texture_id });
	}
}

void UNRegion::greedy_design(TMap<int, unsigned int> &floors, const int &chunk_idx, const int &chunk_z) {
	using namespace nfu;
	auto n_floors = floors.Num();
	const int base_x = Chunks[chunk_idx].base_x;
	const int base_y = Chunks[chunk_idx].base_y;
	const int base_z = Chunks[chunk_idx].base_z;

	while (!floors.Num()==0) {
		const auto first_floor = floors.CreateIterator();
		const auto base_region_idx = first_floor->Key;
		const auto texture_id = first_floor->Value;

		const auto tt = idxmap(base_region_idx);
		const auto tile_x = tt.Get<0>();
		const auto tile_y = tt.Get<1>();
		const auto tile_z = tt.Get<2>();
		auto width = 1;
		auto height = 1;

		floors.Remove(base_region_idx);
		auto idx_grow_right = base_region_idx + 1;
		auto x_coordinate = tile_x;
		auto right_finder = floors.Find(idx_grow_right);
		while (x_coordinate < REGION_WIDTH - 1 && idx_grow_right < mapidx(FMath::Min(REGION_WIDTH - 1, base_x + CHUNK_SIZE), tile_y, tile_z) && right_finder != nullptr && *right_finder == texture_id) {
			floors.Remove(idx_grow_right);
			++width;
			++idx_grow_right;
			++x_coordinate;
			right_finder = floors.Find(idx_grow_right);
		}

		if (tile_y < REGION_HEIGHT - 1) {
			auto y_progress = tile_y + 1;

			auto possible = true;
			while (possible && y_progress < base_y + CHUNK_SIZE && y_progress < REGION_HEIGHT - 1) {
				for (auto gx = tile_x; gx < tile_x + width; ++gx) {
					const auto candidate_idx = mapidx(gx, y_progress, tile_z);
					const auto vfinder = floors.Find(candidate_idx);
					if (vfinder == nullptr || *vfinder != texture_id) possible = false;
				}
				if (possible) {
					++height;
					for (auto gx = tile_x; gx < tile_x + width; ++gx) {
						const auto candidate_idx = mapidx(gx, y_progress, tile_z);
						floors.Remove(candidate_idx);
					}
				}

				++y_progress;
			}
		}

		Chunks[chunk_idx].layers[chunk_z].design_mode.Emplace(floor_t{ tile_x, tile_y, tile_z, width, height, texture_id });
	}
}

void UNRegion::greedy_cubes(TMap<int, unsigned int> &cubes, const int &chunk_idx, const int &chunk_z) {
	using namespace nfu;
	const int base_x = Chunks[chunk_idx].base_x;
	const int base_y = Chunks[chunk_idx].base_y;
	const int base_z = Chunks[chunk_idx].base_z;

	auto n_cubes = cubes.Num();
	while (!cubes.Num()==0) {
		const auto first_floor = cubes.CreateIterator();
		const auto base_region_idx = first_floor->Key;
		const auto texture_id = first_floor->Value;

		const auto tt = idxmap(base_region_idx);
		const auto tile_x = tt.Get<0>();
		const auto tile_y = tt.Get<1>();
		const auto tile_z = tt.Get<2>();
		auto width = 1;
		auto height = 1;

		cubes.Remove(base_region_idx);
		auto idx_grow_right = base_region_idx + 1;
		auto x_coordinate = tile_x;
		auto right_finder = cubes.Find(idx_grow_right);
		while (x_coordinate < REGION_WIDTH - 1 && idx_grow_right < mapidx(FMath::Min(REGION_WIDTH - 1, base_x + CHUNK_SIZE), tile_y, tile_z) && right_finder != nullptr && *right_finder == texture_id) {
			cubes.Remove(idx_grow_right);
			++width;
			++idx_grow_right;
			++x_coordinate;
			right_finder = cubes.Find(idx_grow_right);
		}

		//std::cout << "Merging " << width << " tiles horizontally\n";

		if (tile_y < REGION_HEIGHT - 1) {
			auto y_progress = tile_y + 1;

			auto possible = true;
			while (possible && y_progress < base_y + CHUNK_SIZE && y_progress < REGION_HEIGHT - 1) {
				for (auto gx = tile_x; gx < tile_x + width; ++gx) {
					const auto candidate_idx = mapidx(gx, y_progress, tile_z);
					const auto vfinder = cubes.Find(candidate_idx);
					if (!(vfinder != nullptr && *vfinder == texture_id)) possible = false;
				}
				if (possible) {
					++height;
					for (int gx = tile_x; gx < tile_x + width; ++gx) {
						const auto candidate_idx = mapidx(gx, y_progress, tile_z);
						cubes.Remove(candidate_idx);
					}
				}

				++y_progress;
			}
		}

		Chunks[chunk_idx].layers[chunk_z].cubes.Emplace(cube_t{ tile_x, tile_y, tile_z, width, height, 1, texture_id });
	}
}

void UNRegion::UpdateChunk(const int &chunk_idx) {
	using namespace nfu;
	using namespace regiondefs;

	for (auto &layer : Chunks[chunk_idx].layers) {
		layer.cubes.Empty();
		layer.floors.Empty();
		layer.design_mode.Empty();
	}
	Chunks[chunk_idx].static_voxel_models.Empty();
	Chunks[chunk_idx].vegetation_models.Empty();

	const int base_x = Chunks[chunk_idx].base_x;
	const int base_y = Chunks[chunk_idx].base_y;
	const int base_z = Chunks[chunk_idx].base_z;


	for (int chunk_z = 0; chunk_z < CHUNK_SIZE; ++chunk_z) {
		const int region_z = chunk_z + base_z;
		TMap<int, unsigned int> floors;
		TMap<int, unsigned int> cubes;
		TMap<int, unsigned int> design_mode;

		for (int chunk_y = 0; chunk_y < CHUNK_SIZE; ++chunk_y) {
			const int region_y = chunk_y + base_y;
			for (int chunk_x = 0; chunk_x < CHUNK_SIZE; ++chunk_x) {
				const int region_x = chunk_x + base_x;
				const int ridx = mapidx(region_x, region_y, region_z);

				const auto tiletype = TileType[ridx];
				design_mode.Add(ridx, get_design_tex(ridx));
				if (tiletype != tile_type::OPEN_SPACE) {
					if (testbit(tile_flags::REVEALED, TileFlags[ridx])) {
						if (tiletype == tile_type::WINDOW) {
							cubes.Add(ridx, -3);
						}
						else if (tiletype == tile_type::FLOOR) {
							floors.Add(ridx, get_floor_tex(ridx));
							//if (farm_designations->farms.find(ridx) != farm_designations->farms.end()) {
							//	chunks[chunk_idx].static_voxel_models[116].push_back(std::make_tuple(region_x, region_y, region_z));
							//}

							if (TileVegetationType[ridx] > 0 && !testbit(tile_flags::CONSTRUCTION, TileFlags[ridx])) {
								Chunks[chunk_idx].vegetation_models.Emplace(TTuple<int, int, int, int, int>((int)TileVegetationType[ridx], (int)TileVegetationLifecycle[ridx], (int)region_x, (int)region_y, (int)region_z));
							}
						}
						else if (tiletype == tile_type::TREE_TRUNK) {
							Chunks[chunk_idx].vegetation_models.Emplace(TTuple<int, int, int, int, int>(-1, 0, (int)region_x, (int)region_y, (int)region_z));
							floors.Add(ridx, -1);
						}
						else if (is_cube(tiletype))
						{
							cubes.Add(ridx, get_cube_tex(ridx));
						}
						else if (tiletype == tile_type::RAMP) {
							// TODO: Handle differently
							cubes.Add(ridx, get_cube_tex(ridx));
						}
						else if (tiletype == tile_type::STAIRS_DOWN) {
							if (!Chunks[chunk_idx].static_voxel_models.Contains(24)) Chunks[chunk_idx].static_voxel_models.Add(24, TArray<TTuple<int, int, int>>());
							Chunks[chunk_idx].static_voxel_models[24].Emplace(TTuple<int,int,int>(region_x, region_y, region_z));
						}
						else if (tiletype == tile_type::STAIRS_UP) {
							if (!Chunks[chunk_idx].static_voxel_models.Contains(34)) Chunks[chunk_idx].static_voxel_models.Add(23, TArray<TTuple<int, int, int>>());
							Chunks[chunk_idx].static_voxel_models[23].Emplace(TTuple<int, int, int>(region_x, region_y, region_z));
						}
						else if (tiletype == tile_type::STAIRS_UPDOWN) {
							if (!Chunks[chunk_idx].static_voxel_models.Contains(25)) Chunks[chunk_idx].static_voxel_models.Add(25, TArray<TTuple<int, int, int>>());
							Chunks[chunk_idx].static_voxel_models[25].Emplace(TTuple<int, int, int>(region_x, region_y, region_z));
						}
						else if (tiletype == tile_type::CLOSED_DOOR) {
							auto vox_id = 128;
							const auto bid = BuildingId[ridx];
							if (bid > 0)
							{
								/*const auto building_entity = bengine::entity(bid);
								if (building_entity)
								{
									const auto building_comp = building_entity->component<building_t>();
									if (building_comp)
									{
										const auto def = get_building_def(building_comp->tag);
										if (def)
										{
											for (const auto &p : def->provides)
											{
												if (p.alternate_vox > 0) vox_id = p.alternate_vox;
											}
										}
									}
								}*/
							}
							if (!Chunks[chunk_idx].static_voxel_models.Contains(vox_id)) Chunks[chunk_idx].static_voxel_models.Add(vox_id, TArray<TTuple<int, int, int>>());
							Chunks[chunk_idx].static_voxel_models[vox_id].Emplace(TTuple<int, int, int>(region_x, region_y, region_z));
						}
					} // revealed
					else {
						cubes.Add(ridx, 3);
					}
				}
			}
		}

		greedy_floors(floors, chunk_idx, chunk_z);
		greedy_cubes(cubes, chunk_idx, chunk_z);
		greedy_design(design_mode, chunk_idx, chunk_z);
	}
}

unsigned int UNRegion::get_design_tex(const int &idx) {
	using namespace regiondefs;
	const auto tt = TileType[idx];

	// Default graphics for open space and not-yet-revealed
	if (tt == tile_type::OPEN_SPACE) return 3;
	if (!testbit(tile_flags::REVEALED, TileFlags[idx])) return 3;
	if (tt == tile_type::FLOOR) return get_floor_tex(idx);
	if (tt == tile_type::TREE_TRUNK) return 6;
	return get_cube_tex(idx);
}

unsigned int UNRegion::get_floor_tex(const int &idx) {
	using namespace regiondefs;

	UNoxGameInstance * game = Cast<UNoxGameInstance>(UGameplayStatics::GetGameInstance(this));
	auto raws = game->GetRaws();

	// If its a stockpile, render it as such
	if (StockpileId[idx] > 0) return 3; // TODO: Determine texture

												 // We no longer hard-code grass.
	if (TileVegetationType[idx] > 0 && !testbit(tile_flags::CONSTRUCTION, TileFlags[idx])) {
		switch (TileVegetationLifecycle[idx]) {
		case 0: return 18; // Germination
		case 1: return 21; // Sprouting
		case 2: return -1; // Growing (grass is material 0)
		case 3: return 24; // Flowering
		}
		return -1; // Grass is determined to be index -1
	}
	const auto material_idx = TileMaterial[idx];
	const auto material = raws->get_material(material_idx);
	if (!material) return -2; // -2 is the super-obvious "we don't have a material" texture.

	unsigned int use_id = -2;
	if (testbit(tile_flags::CONSTRUCTION, TileFlags[idx])) {
		use_id = (unsigned int)material->floor_smooth_id;
	}
	else {
		use_id = (unsigned int)material->floor_rough_id;
	}
	if (use_id == 3) {
		//glog(log_target::LOADER, log_severity::warning, "Material [{0}] is lacking a texture", material->name);
	}

	//std::cout << material->name << "=" << use_id << "\n";
	return use_id;
}

unsigned int UNRegion::get_cube_tex(const int &idx) {
	using namespace regiondefs;

	UNoxGameInstance * game = Cast<UNoxGameInstance>(UGameplayStatics::GetGameInstance(this));
	auto raws = game->GetRaws();

	const auto tt = TileType[idx];
	if (tt == tile_type::TREE_TRUNK) return 6;
	if (tt == tile_type::TREE_LEAF) return 9;

	const auto material_idx = TileMaterial[idx];
	const auto material = raws->get_material(material_idx);
	if (!material) return -2;

	unsigned int use_id = -2;
	if (!testbit(tile_flags::CONSTRUCTION, TileFlags[idx])) {
		use_id = (unsigned int)material->wall_smooth_id;
	}
	else {
		use_id = (unsigned int)material->wall_rough_id;
	}
	if (use_id == -2) {
		//glog(log_target::LOADER, log_severity::warning, "Material [{0}] is lacking a texture.", material->name);
	}
	return use_id;
}

void UNRegion::TileRecalcAll() {
	using namespace nfu;

	for (int z = 0; z < REGION_DEPTH; ++z) {
		for (int y = 0; y < REGION_HEIGHT; ++y) {
			for (int x = 0; x < REGION_WIDTH; ++x) {
				TileCalculate(x, y, z);
			}
		}
	}
	for (int z = 0; z < REGION_DEPTH; ++z) {
		for (int y = 0; y < REGION_HEIGHT; ++y) {
			for (int x = 0; x < REGION_WIDTH; ++x) {
				TilePathing(x, y, z);
			}
		}
	}
}

void UNRegion::TileCalculate(const int &x, const int &y, const int &z) {
	using namespace nfu;
	using namespace regiondefs;

	const auto idx = mapidx(x, y, z);

	// Calculate render characteristics
	//calc_render(idx);

	// Solidity and first-pass standability
	if (TileType[idx] == tile_type::SEMI_MOLTEN_ROCK || TileType[idx] == tile_type::SOLID
		|| TileType[idx] == tile_type::WALL || TileType[idx] == tile_type::TREE_TRUNK ||
		TileType[idx] == tile_type::TREE_LEAF
		|| TileType[idx] == tile_type::WINDOW || TileType[idx] == tile_type::CLOSED_DOOR)
	{
		setbit(tile_flags::SOLID, TileFlags[idx]);
		if (TileType[idx] == tile_type::WINDOW) {
			resetbit(tile_flags::OPAQUE_TILE, TileFlags[idx]);
		}
		else {
			setbit(tile_flags::OPAQUE_TILE, TileFlags[idx]);
		}
		resetbit(tile_flags::CAN_STAND_HERE, TileFlags[idx]);
	}
	else {
		resetbit(tile_flags::SOLID, TileFlags[idx]);

		// Locations on which one can stand
		setbit(tile_flags::CAN_STAND_HERE, TileFlags[idx]);
		if (TileType[idx] == tile_type::OPEN_SPACE) resetbit(tile_flags::CAN_STAND_HERE, TileFlags[idx]);

		if (z > 0) {
			const auto idx_below = mapidx(x, y, z - 1);

			// Can stand on the tile above walls, ramps and up stairs
			if (TileType[idx] == tile_type::FLOOR) setbit(tile_flags::CAN_STAND_HERE, TileFlags[idx]);
			if (TileType[idx] == tile_type::OPEN_SPACE && TileType[idx_below] == tile_type::WALL)
				setbit(tile_flags::CAN_STAND_HERE, TileFlags[idx]);
			if (TileType[idx] == tile_type::OPEN_SPACE && TileType[idx_below] == tile_type::RAMP)
				setbit(tile_flags::CAN_STAND_HERE, TileFlags[idx]);
			if (TileType[idx] == tile_type::OPEN_SPACE && TileType[idx_below] == tile_type::STAIRS_UP)
				setbit(tile_flags::CAN_STAND_HERE, TileFlags[idx]);
			if (TileType[idx] == tile_type::OPEN_SPACE && TileType[idx_below] == tile_type::STAIRS_UPDOWN)
				setbit(tile_flags::CAN_STAND_HERE, TileFlags[idx]);
			if (TileType[idx] == tile_type::OPEN_SPACE && TileType[idx_below] == tile_type::SOLID)
				setbit(tile_flags::CAN_STAND_HERE, TileFlags[idx]);
		}
	}

	TilePathing(x, y, z);
}

void UNRegion::TilePathing(const int &x, const int &y, const int &z) {
	using namespace nfu;
	using namespace regiondefs;

	const auto idx = mapidx(x, y, z);

	// Start with a clean slate
	resetbit(tile_flags::CAN_GO_NORTH, TileFlags[idx]);
	resetbit(tile_flags::CAN_GO_SOUTH, TileFlags[idx]);
	resetbit(tile_flags::CAN_GO_EAST, TileFlags[idx]);
	resetbit(tile_flags::CAN_GO_EAST, TileFlags[idx]);
	resetbit(tile_flags::CAN_GO_UP, TileFlags[idx]);
	resetbit(tile_flags::CAN_GO_DOWN, TileFlags[idx]);
	resetbit(tile_flags::CAN_GO_NORTH_EAST, TileFlags[idx]);
	resetbit(tile_flags::CAN_GO_NORTH_WEST, TileFlags[idx]);
	resetbit(tile_flags::CAN_GO_SOUTH_EAST, TileFlags[idx]);
	resetbit(tile_flags::CAN_GO_SOUTH_WEST, TileFlags[idx]);

	if (testbit(tile_flags::SOLID, TileFlags[idx]) || !testbit(tile_flags::CAN_STAND_HERE, TileFlags[idx])) {
		// If you can't go there, it doesn't have any exits.
	}
	else {
		if (x > 0 && testbit(tile_flags::CAN_STAND_HERE, TileFlags[mapidx(x-1, y, z)])) setbit(tile_flags::CAN_GO_WEST, TileFlags[idx]);
		if (x < REGION_WIDTH - 1 && testbit(tile_flags::CAN_STAND_HERE, TileFlags[mapidx(x + 1, y, z)])) setbit(tile_flags::CAN_GO_EAST, TileFlags[idx]);
		if (y > 0 && testbit(tile_flags::CAN_STAND_HERE, TileFlags[mapidx(x, y-1, z)])) setbit(tile_flags::CAN_GO_NORTH, TileFlags[idx]);
		if (y < REGION_HEIGHT - 1 && testbit(tile_flags::CAN_STAND_HERE, TileFlags[mapidx(x, y+1, z)])) setbit(tile_flags::CAN_GO_SOUTH, TileFlags[idx]);
		if (x > 0 && y > 0 && testbit(tile_flags::CAN_STAND_HERE, TileFlags[mapidx(x - 1, y - 1, z)])) setbit(tile_flags::CAN_GO_NORTH_WEST, TileFlags[idx]);
		if (x < REGION_WIDTH - 1 && y > 0 && testbit(tile_flags::CAN_STAND_HERE, TileFlags[mapidx(x + 1, y - 1, z)])) setbit(tile_flags::CAN_GO_NORTH_EAST, TileFlags[idx]);
		if (x > 0 && y < REGION_HEIGHT - 1 && testbit(tile_flags::CAN_STAND_HERE, TileFlags[mapidx(x - 1, y + 1, z)])) setbit(tile_flags::CAN_GO_SOUTH_WEST, TileFlags[idx]);
		if (x < REGION_WIDTH - 1 && y < REGION_HEIGHT - 1 && testbit(tile_flags::CAN_STAND_HERE, TileFlags[mapidx(x + 1, y + 1, z)])) setbit(tile_flags::CAN_GO_SOUTH_EAST, TileFlags[idx]);

		if (z < REGION_DEPTH - 1 &&
			(TileType[idx] == tile_type::RAMP || TileType[idx] == tile_type::STAIRS_UP || TileType[idx] == tile_type::STAIRS_UPDOWN) && testbit(tile_flags::CAN_STAND_HERE, TileFlags[idx])) {
			setbit(tile_flags::CAN_GO_UP, TileFlags[idx]);
		}

		if (z > 0 && (TileType[idx] == tile_type::STAIRS_DOWN || TileType[idx] == tile_type::STAIRS_UPDOWN) && testbit(tile_flags::CAN_STAND_HERE, TileFlags[idx])) {
			setbit(tile_flags::CAN_GO_DOWN, TileFlags[idx]);
		}

		if (z > 0 && TileType[idx] == tile_type::OPEN_SPACE && TileType[mapidx(x, y, z - 1)] == tile_type::RAMP) {
			setbit(tile_flags::CAN_GO_DOWN, TileFlags[idx]);
		}
	}
}
