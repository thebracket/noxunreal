// (c) 2016 - Present, Bracket Productions

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "../BEngine/RandomNumberGenerator.h"
#include "../BEngine/FastNoise.h"
#include "NPlanet.generated.h"

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

USTRUCT(BlueprintType)
struct FNPlanetBlock
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
	uint8 height = 0;

	UPROPERTY(BlueprintReadOnly)
	uint8 variance = 0;

	UPROPERTY(BlueprintReadOnly)
	uint8 type = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 temperature_c = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 rainfall = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 biome_idx = -1;

	uint32 RiverMask = 0;
	uint32 Features = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 OwnerCiv = -1;
};

USTRUCT(BlueprintType)
struct FNBiome
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 type = 0;

	UPROPERTY(BlueprintReadOnly)
	FString name = "";

	UPROPERTY(BlueprintReadOnly)
	int32 mean_temperature = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 mean_rainfall = 0;

	UPROPERTY(BlueprintReadOnly)
	uint8 mean_altitude = 0;

	UPROPERTY(BlueprintReadOnly)
	uint8 mean_variance = 0;

	UPROPERTY(BlueprintReadOnly)
	uint8 warp_mutation = 0;

	UPROPERTY(BlueprintReadOnly)
	uint8 evil = 0;

	UPROPERTY(BlueprintReadOnly)
	uint8 savagery = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 center_x = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 center_y = 0;

	UPROPERTY(BlueprintReadOnly)
	FString BiomeTypeName;
};

USTRUCT(BlueprintType)
struct FNRiverStep
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 x = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 y = 0;
};

USTRUCT(BlueprintType)
struct FNRiver
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString name = "A river that will one day have a name";

	UPROPERTY(BlueprintReadOnly)
	int32 start_x = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 start_y = 0;

	UPROPERTY(BlueprintReadOnly)
	TArray<FNRiverStep> steps;
};

USTRUCT(BlueprintType)
struct FNCivilization {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString SpeciesTag;

	UPROPERTY(BlueprintReadOnly)
	int StartX;

	UPROPERTY(BlueprintReadOnly)
	int StartY;

	UPROPERTY(BlueprintReadOnly)
	FLinearColor Color;

	UPROPERTY(BlueprintReadOnly)
	bool Extinct = false;

	UPROPERTY(BlueprintReadOnly)
	TMap<int32, int32> Relations;
};

USTRUCT(BlueprintType)
struct FSettlement {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
	int Civilization;

	UPROPERTY(BlueprintReadOnly)
	int x;

	UPROPERTY(BlueprintReadOnly)
	int y;

	UPROPERTY(BlueprintReadOnly)
	int size;

	UPROPERTY(BlueprintReadOnly)
	int FoodStock = 0;

	UPROPERTY(BlueprintReadOnly)
	TArray<int> DevelopedTiles;

};

/**
 * Holds basic information about the whole planet.
 */
UCLASS()
class NOXUNREAL_API UNPlanet : public UObject
{
	GENERATED_BODY()
	
public:
	UNPlanet() {
	}

	UPROPERTY(BlueprintReadOnly)
	TArray<FNCivilization> civilizations;

	UPROPERTY(BlueprintReadOnly)
	TMap<int, FSettlement> settlements;

	UPROPERTY(BlueprintReadOnly)
	FString name = "Test World";

	UPROPERTY(BlueprintReadOnly)
	int32 RngSeed;

	UPROPERTY(BlueprintReadOnly)
	int32 PerlinSeed;

	UPROPERTY(BlueprintReadOnly)
	int32 RemainingSettlers;

	UPROPERTY(BlueprintReadOnly)
	int32 MigrantCounter;

	UPROPERTY(BlueprintReadOnly)
	int32 WaterDivisor = 3;

	UPROPERTY(BlueprintReadOnly)
	int32 PlaintsDivisor = 3;

	UPROPERTY(BlueprintReadOnly)
	int32 StartingSettlers = 10;

	UPROPERTY(BlueprintReadOnly)
	bool StrictBeamdown = false;

	UPROPERTY(BlueprintReadOnly)
	uint8 water_height;

	UPROPERTY(BlueprintReadOnly)
	uint8 plains_height;

	UPROPERTY(BlueprintReadOnly)
	uint8 hills_height;

	UPROPERTY(BlueprintReadOnly)
	TArray<FNPlanetBlock> Landblocks;

	UPROPERTY(BlueprintReadOnly)
	TArray<FNBiome> Biomes;

	UPROPERTY(BlueprintReadOnly)
	TArray<FNRiver> Rivers;
	
	// TODO: CivHolder and History

	constexpr inline int idx(const int x, const int y) const { return y * nfu::WORLD_WIDTH + x; }

	UFUNCTION(BlueprintCallable)
	void BuildPlanet(const int seed, const int water_divisor, const int plains_divisor, const int starting_settlers, const bool strict_beamdown);

	UFUNCTION(BlueprintCallable)
	FNPlanetBlock GetWorldBlock(const int x, const int y);

	UFUNCTION(BlueprintCallable)
	FNBiome GetWorldBiome(const int x, const int y);

	RandomNumberGenerator * GetRNG() { return &rng; }

	FastNoise GetNoiseMap() { return noiseMap; }

	UPROPERTY(BlueprintReadOnly)
	int32 Year = 0;

	UFUNCTION(BlueprintCallable)
	void RunWorldgenYear();

private:
	const int n_civs = 128;

	FastNoise noiseMap;
	RandomNumberGenerator rng;
	void ZeroFillPlanet();
	FastNoise PlanetNoiseMap();
	void BaseTypeAllocation();
	int32 DetermineProportion(int32 &candidate, int32 target);
	void MarkCoastlines();
	void Rainfall();
	void BuildBiomes(RandomNumberGenerator &rng);
	TMap<uint8, double> BiomeMembership(const int32_t &idx);
	TArray<TPair<double, size_t>> FindPossibleBiomes(TMap<uint8, double> &percents, const FNBiome &biome);
	FString NameBiome(RandomNumberGenerator &rng, FNBiome &biome);
	void RunRivers(RandomNumberGenerator &rng);
	void BuildCivilizations(RandomNumberGenerator &rng);
	void PlaceStartingCivs(RandomNumberGenerator &rng);
};
