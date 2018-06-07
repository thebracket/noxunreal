// (c) 2016 - Present, Bracket Productions

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "../BEngine/RandomNumberGenerator.h"
#include "NPlanet.h"
#include "GameComponents.h"
#include "../BEngine/bittools.h"
#include "regiondefs.h"
#include "chunking.h"
#include "NRegion.generated.h"

/**
 * Provides information and generation functions for a region.
 */
UCLASS()
class NOXUNREAL_API UNRegion : public UObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void SetupEmptyRegion();

	UFUNCTION(BlueprintCallable)
	void BuildRegion(const int StartX, const int StartY, const bool StartingArea);

	UFUNCTION(BlueprintCallable)
	void SetPlanet(UNPlanet * p) { planet = p; }

	UPROPERTY(BlueprintReadOnly)
	int RegionX = 0;

	UPROPERTY(BlueprintReadOnly)
	int RegionY = 0;

	UPROPERTY(BlueprintReadOnly)
	int BiomeIndex = 0;

	/*
	* Calculate the array index of an x/y/z position.
	*/
	inline int mapidx(const int &x, const int &y, const int &z) noexcept {
		return (z * nfu::REGION_HEIGHT * nfu::REGION_WIDTH) + (y * nfu::REGION_WIDTH) + x;
	}

	inline TTuple<int, int, int> idxmap(int idx) noexcept {
		int z = idx / (nfu::REGION_HEIGHT * nfu::REGION_WIDTH);
		idx -= (z * nfu::REGION_WIDTH * nfu::REGION_HEIGHT);

		int y = idx / nfu::REGION_WIDTH;
		idx -= (y * nfu::REGION_WIDTH);

		int x = idx;

		return TTuple<int, int, int>(x, y, z);
	}

	inline void SetTileType(const int idx, const uint8_t type) {
		TileType[idx] = type;
	}

	inline void SetTileMaterial(const int idx, const size_t material) {
		TileMaterial[idx] = material;
	}

	inline void reveal(const int idx) {
		setbit(regiondefs::tile_flags::REVEALED, TileFlags[idx]);
	}

	inline void SetWaterLevel(const int idx, const uint32_t level) {
		WaterLevel[idx] = level;
	}

	/* Set the vegetation type index for a cell. */
	inline void SetVegType(const int idx, const uint8_t type) {
		TileVegetationType[idx] = type;
	}

	/* Set the vegetation hit points for a cell. */
	inline void SetVegHp(const int idx, const uint8_t hp) {
		VegetationHitPoints[idx] = hp;
	}

	/* Set the vegetation tick counter for a cell. */
	inline void SetVegTicker(const int idx, const uint16_t ticker) {
		TileVegetationTicker[idx] = ticker;
	}

	/* Set the vegetation lifecycle state for a cell. */
	inline void SetVegLifecycle(const int idx, const uint8_t lifecycle) {
		TileVegetationLifecycle[idx] = lifecycle;
	}

	inline bool flag(const int idx, const int flag) {
		return testbit(flag, TileFlags[idx]);
	}

	/* Find ground-level (Z) for a given X/Y pair */
	inline int GroundZ(const int x, const int y) {
		int z = nfu::REGION_DEPTH - 1;
		bool hit_ground = false;
		while (!hit_ground) {
			const auto idx = mapidx(x, y, z);
			if (TileType[idx] == regiondefs::tile_type::SOLID) {
				hit_ground = true;
				++z;
			}
			else {
				--z;
			}
			if (z == 1) hit_ground = true;
		}
		return z;
	}

	inline bool CanSeeSky(const int &x, const int &y, const int &z) noexcept {
		auto result = true;

		auto Z = z;
		while (Z < nfu::REGION_DEPTH) {
			if (flag(mapidx(x, y, Z), regiondefs::tile_flags::SOLID)) result = false;
			Z++;
		}
		return result;
	}

	inline void set_tile_type(const int idx, const uint8_t type) {
		if (idx < 0 || idx > nfu::REGION_TILES_COUNT) return;
		TileType[idx] = type;
	}

	inline void set_tile(const int idx, const uint8_t type, const bool solid, const bool opaque,
		const std::size_t material, const uint8_t water, const bool remove_vegetation,
		const bool construction) 
	{
		set_tile_type(idx, type);
		if (solid) {
			setbit(regiondefs::tile_flags::SOLID, TileFlags[idx]);
		}
		else
		{
			resetbit(regiondefs::tile_flags::SOLID, TileFlags[idx]);
		}
		if (opaque)
		{
			setbit(regiondefs::tile_flags::OPAQUE_TILE, TileFlags[idx]);
		}
		else
		{
			resetbit(regiondefs::tile_flags::OPAQUE_TILE, TileFlags[idx]);
		}
		SetTileMaterial(idx, material);
		if (remove_vegetation) TileVegetationType[idx] = 0;
		WaterLevel[idx] = water;
		if (construction) setbit(regiondefs::tile_flags::CONSTRUCTION, TileFlags[idx]);
	}

	TArray<chunk_t> Chunks;
	TBitArray<> DirtyChunks;
	void InitializeChunks();
	void UpdateChunks();
	void SetupChunk(const int &idx, const int &x, const int &y, const int &z);
	inline void MarkChunkDirty(const int &idx) { DirtyChunks[idx] = true; }
	void UpdateChunk(const int &chunk_idx);

	void TileRecalcAll();
	void TileCalculate(const int &x, const int &y, const int &z);
	void TilePathing(const int &x, const int &y, const int &z);

	UNPlanet * planet;

	// Region Tile Data
	TArray<uint8> TileType;
	TArray<uint16> TileMaterial;
	TArray<uint16> HitPoints;
	TArray<uint8> VegetationHitPoints;
	TArray<uint32> BuildingId;
	TArray<uint32> TreeId;
	TArray<uint32> BridgeId;
	TArray<uint16> TileVegetationType;
	TArray<uint16> TileVegetationTicker;
	TArray<uint8> TileVegetationLifecycle;
	TArray<uint32> StockpileId;
	TArray<uint32> TileFlags;
	TArray<uint32> WaterLevel;
	int NextTreeID;

	// Internal
	TArray<size_t> soils;
	TArray<size_t> sands;
	TArray<size_t> sedimintaries;
	TArray<size_t> igneouses;

	// Builders
	TArray<uint8> CreateEmptyHeightmap();
	void BuildHeightmapFromNoise(TArray<uint8> &HeightMap);
	TArray<int32> CreateSubregions(TArray<uint8> &HeightMap, RandomNumberGenerator * rng, TArray<uint8> &PooledWater, TArray<TPair<int, uint8>> &WaterSpawners);
	void JustAddWater(TArray<uint8> &HeightMap, RandomNumberGenerator * rng, TArray<uint8> &PooledWater, TArray<TPair<int, uint8>> &WaterSpawners);
	regiondefs::strata_t BuildStrata(TArray<uint8> &HeightMap, RandomNumberGenerator * rng);
	void LayStrata(TArray<uint8> &HeightMap, RandomNumberGenerator * rng, TArray<uint8> &PooledWater, TArray<TPair<int, uint8>> &WaterSpawners, regiondefs::strata_t &strata);
	void BuildRamps(const TArray<uint8> &PooledWater);
	void BuildBeaches();
	void BuildTrees(RandomNumberGenerator * rng, bool StartingArea);
	void BuildDebrisTrail(const int &crash_x, const int &crash_y);
	void BuildEscapePod(const int &crash_x, const int &crash_y, const int &crash_z);
	void BuildGameComponents(const int &crash_x, const int &crash_y, const int &crash_z);
	void add_construction(const int x, const int y, const int z, const FString type, bool solid, const size_t &civ_owner) noexcept;
	void add_building(FString tag, const int x, const int y, const int z, const size_t &civ_owner) noexcept;
	int CreateSettler(const int x, const int y, const int z, RandomNumberGenerator * rng, int shift_id) noexcept;
	TArray<FString> get_event_candidates(const int &age, const TArray<FString> &past) noexcept;
	void BuildRoads(TSet<int> &used_tiles);
	void BuildFarms(RandomNumberGenerator *rng, TSet<int> &used_tiles);
	void BuildMines(RandomNumberGenerator *rng, TSet<int> &used_tiles);
	void BuildHuts(RandomNumberGenerator *rng, TSet<int> &used_tiles);
	void AddBuilding(FString path, int x, int y, TSet<int> &used_tiles, RandomNumberGenerator * rng);

	void decorate_item_categories(int &item, TBitArray<> &categories) noexcept;
	void spawn_item_in_container(const int container_id, const FString &tag, const std::size_t &material, uint8_t quality, uint8_t wear, int creator_id, FString creator_name) noexcept;
	void spawn_item_carried(const int holder_id, const FString &tag, const std::size_t &material, const ecs_item_location_t &loc, uint8_t quality, uint8_t wear, int creator_id, FString creator_name, RandomNumberGenerator * rng) noexcept;

	// Chunking
	unsigned int get_design_tex(const int &idx);
	unsigned int get_floor_tex(const int &idx);
	unsigned int get_cube_tex(const int &idx);
	void greedy_floors(TMap<int, unsigned int> &floors, const int &chunk_idx, const int &chunk_z);
	void greedy_design(TMap<int, unsigned int> &floors, const int &chunk_idx, const int &chunk_z);
	void greedy_cubes(TMap<int, unsigned int> &cubes, const int &chunk_idx, const int &chunk_z);
};
