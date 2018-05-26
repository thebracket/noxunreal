// (c) 2016 - Present, Bracket Productions

#include "NRegion.h"
#include "../Public/NoxGameInstance.h"
#include "../Raws/NRaws.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

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

	// TODO: Build game components
	
	// Trees and Blight
	// TODO: Determine blight level
	const int blight_level = 0;
	if (blight_level < 100) {
		BuildTrees(rng, StartingArea);
	}

	// Debris Trail
	if (StartingArea) BuildDebrisTrail(crash_x, crash_y);

	// Build Escape Pod
	// Add Settlers
	// Add Features
	// Recalculate all tiles
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
					dig_targets[idx] = region_water_feature_tile{ idx, has_water, depth, heightmap[idx] };
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
