// (c) 2016 - Present, Bracket Productions

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "../BEngine/RandomNumberGenerator.h"
#include "../BEngine/FastNoise.h"
#include "../Raws/NRaws.h"
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

namespace nfu {

	constexpr int WORLD_HEIGHT = 128;
	constexpr int WORLD_WIDTH = 128;
	constexpr int WORLD_TILES_COUNT = WORLD_HEIGHT * WORLD_WIDTH;

	constexpr int REGION_WIDTH = 256;
	constexpr int REGION_HEIGHT = 256;
	constexpr int REGION_DEPTH = 128;
	constexpr int REGION_TILES_COUNT = REGION_WIDTH * REGION_HEIGHT * REGION_DEPTH;
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

/**
 * Holds basic information about the whole planet.
 */
UCLASS()
class NOXUNREAL_API UNPlanet : public UObject
{
	GENERATED_BODY()
	
public:
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

	constexpr inline int idx(const int x, const int y) { return y * nfu::WORLD_WIDTH + x; }

	void BuildPlanet(const int &seed, const int &water_divisor, const int &plains_divisor, const int &starting_settlers, const bool &strict_beamdown);
	NRaws * raws;

private:
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
};
