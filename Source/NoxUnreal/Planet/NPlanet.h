// (c) 2016 - Present, Bracket Productions

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "../BEngine/RandomNumberGenerator.h"
#include "../BEngine/FastNoise.h"
#include "constants.h"
#include "NPlanet.generated.h"

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

	UPROPERTY(BlueprintReadOnly)
	int32 OwnerSettlement = -1;
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
struct FUnit {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 UnitType;

	UPROPERTY(BlueprintReadOnly)
	int32 MaxStrength;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentStrength;
};

USTRUCT(BlueprintType)
struct FArmy {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
	TArray<FUnit> units;

	UPROPERTY(BlueprintReadOnly)
	int victories = 0;
};

USTRUCT(BlueprintType)
struct FNCivilization {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString SpeciesTag;

	UPROPERTY(BlueprintReadOnly)
	int GovernmentIndex;

	UPROPERTY(BlueprintReadOnly)
	int TechLevel = 1;

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

	UPROPERTY(BlueprintReadOnly)
	float FoodWealth = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float MineralWealth = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float LumberWealth = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float CashWealth = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	TArray<FArmy> armies;
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
	int CashStock = 0;

	UPROPERTY(BlueprintReadOnly)
	int LumberStock = 0;

	UPROPERTY(BlueprintReadOnly)
	int MineralStock = 0;

	UPROPERTY(BlueprintReadOnly)
	TArray<int> DevelopedTiles;

	UPROPERTY(BlueprintReadOnly)
	TSet<int> TradeRoutes;

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
	inline FVector2D idxmap(const int index) {
		FVector2D result;
		result.X = index % nfu::WORLD_WIDTH;
		result.Y = index / nfu::WORLD_WIDTH;
		return result;
	}

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
	const int n_civs = 32;

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
	int DetermineInitialRelationship(const int &civ1, const int &civ2);
};
