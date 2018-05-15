// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NoxRegionChunk.h"
#include "NoxDynamicLight.h"
#include "ProceduralMeshComponent.h"
#include "../../ThirdParty/libnox/Includes/libnox.h"
#include "NoxCursors.h"
#include "NoxLinkTest.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPowerChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCashChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDateChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPauseModeChanged);

USTRUCT(BlueprintType)
struct FNoxSettlerListEntry
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString name;

	UPROPERTY(BlueprintReadOnly)
	FString gender;

	UPROPERTY(BlueprintReadOnly)
	FString profession;

	UPROPERTY(BlueprintReadOnly)
	FString task;

	UPROPERTY(BlueprintReadOnly)
	FString hp;

	UPROPERTY(BlueprintReadOnly)
	float hp_percent;

	UPROPERTY(BlueprintReadOnly)
	int id;
};

USTRUCT(BlueprintType)
struct FTooltipBlock 
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString tooltip;
};

USTRUCT(BlueprintType)
struct FSettlerJob
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
		FString name;

	UPROPERTY(BlueprintReadOnly)
		FString profession;

	UPROPERTY(BlueprintReadOnly)
		bool isMiner;

	UPROPERTY(BlueprintReadOnly)
		bool isLumberjack;

	UPROPERTY(BlueprintReadOnly)
		bool isFarmer;

	UPROPERTY(BlueprintReadOnly)
		bool isHunter;

	UPROPERTY(BlueprintReadOnly)
		int id;
};

USTRUCT(BlueprintType)
struct FAvailableBuilding
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString name;

	UPROPERTY(BlueprintReadOnly)
	FString tag;
};

USTRUCT(BlueprintType)
struct FPlantableSeed
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
	int number;

	UPROPERTY(BlueprintReadOnly)
	FString name;

	UPROPERTY(BlueprintReadOnly)
	FString grows_into;
};

UCLASS()
class NOXUNREAL_API ANoxLinkTest : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANoxLinkTest();

	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent * mesh;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PostLoad() override;
	virtual void PostActorCreated() override;


	void UpdateCashDisplay(const nf::hud_info_t &hudLink);
	void UpdatePowerDisplay(const nf::hud_info_t &hudLink);
	void UpdateCalendar(const nf::hud_info_t &hudLink);
	int LastPauseMode = -1;
	int LastCash = -1;
	int LastPowerC = -1;
	int LastPowerM = -1;
	int LastMinute = -1;
	bool firstTick = true;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	TArray<ANoxRegionChunk *> RegionChunks;
	TMap<int, TMap<int, ANoxStaticModel *>> VoxModels;
	TMap<int, ANoxDynamicLight *> DynamicLights;
	ANoxCursors * Cursors;

	UPROPERTY(BlueprintReadOnly)
	float PowerPercent;

	UPROPERTY(BlueprintReadOnly)
	FString PowerDisplay;

	UPROPERTY(BlueprintReadOnly)
	FString CashDisplay;

	UPROPERTY(BlueprintReadOnly)
	FString DateDisplay;

	UPROPERTY(BlueprintReadOnly)
	FString GameRunMode;

	UPROPERTY(BlueprintReadOnly)
	float SunRotation;

	UFUNCTION(BlueprintCallable)
	void PauseGame();

	UFUNCTION(BlueprintCallable)
	void RunGame();

	UFUNCTION(BlueprintCallable)
	void SingleStepGame();

	// To handle tooltips
	UFUNCTION(BlueprintCallable)
	void SetWorldPositionFromMouse(FVector vec);

	UFUNCTION(BlueprintCallable)
	void PopulateTooltip();

	UPROPERTY(BlueprintReadOnly)
	FTooltipBlock TooltipBlock;

	UPROPERTY(BlueprintReadOnly)
	FTooltipBlock FarmYield;

	UFUNCTION(BlueprintCallable)
	void GetUnitLists();

	// For the settler unit list box
	UPROPERTY(BlueprintReadOnly)
	TArray<FNoxSettlerListEntry> SettlerList;

	UPROPERTY(BlueprintReadOnly)
	TArray<FNoxSettlerListEntry> NativeList;

	UPROPERTY(BlueprintReadOnly)
	TArray<FNoxSettlerListEntry> WildlifeList;

	UPROPERTY(BlueprintReadOnly)
	TArray<FSettlerJob> SettlerJobList;

	UFUNCTION(BlueprintCallable)
	void ZoomSettler(int id);

	UFUNCTION(BlueprintCallable)
	void FollowSettler(int id);

	UFUNCTION(BlueprintCallable)
	void UpdateSettlerJobList();

	UFUNCTION(BlueprintCallable)
	void MakeMiner(int id);

	UFUNCTION(BlueprintCallable)
	void MakeFarmer(int id);

	UFUNCTION(BlueprintCallable)
	void MakeLumberjack(int id);

	UFUNCTION(BlueprintCallable)
	void FireMiner(int id);

	UFUNCTION(BlueprintCallable)
	void FireFarmer(int id);

	UFUNCTION(BlueprintCallable)
	void FireLumberjack(int id);

	UFUNCTION(BlueprintCallable)
	void MakeHunter(int id);

	UFUNCTION(BlueprintCallable)
	void FireHunter(int id);

	UFUNCTION(BlueprintCallable)
	void SetGameMode(int major, int minor);

	UFUNCTION(BlueprintCallable)
	void GuardModeSet();

	UFUNCTION(BlueprintCallable)
	void GuardModeClear();

	UFUNCTION(BlueprintCallable)
	void LumberjackModeSet();

	UFUNCTION(BlueprintCallable)
	void LumberjackModeClear();

	UFUNCTION(BlueprintCallable)
	void RefreshBuildableList();

	UPROPERTY(BlueprintReadOnly)
	TArray<FAvailableBuilding> AvailableBuildings;

	UPROPERTY(BlueprintReadWrite)
	bool HasSelectedBuilding;

	UFUNCTION(BlueprintCallable)
	void SetBuildingTarget(FString name);

	UFUNCTION(BlueprintCallable)
	void PlaceBuilding();

	UFUNCTION(BlueprintCallable)
	void PopulateFarmYield();

	UFUNCTION(BlueprintCallable)
	void HarvestModeSet();

	UFUNCTION(BlueprintCallable)
	void HarvestModeClear();

	UFUNCTION(BlueprintCallable)
	void RefreshPlantableList();

	UPROPERTY(BlueprintReadOnly)
	TArray<FPlantableSeed> PlantableSeeds;

	UFUNCTION(BlueprintCallable)
	void SetPlantingTarget(FString name);

	UFUNCTION(BlueprintCallable)
	void SetPlanting();

	UFUNCTION(BlueprintCallable)
	void ClearPlanting();

	UFUNCTION(BlueprintCallable)
	int GetMiningMode();

	UFUNCTION(BlueprintCallable)
	void SetMiningMode(int n);

	UFUNCTION(BlueprintCallable)
	void PlaceMine();

	UFUNCTION(BlueprintCallable)
	void ClearMine();

	UFUNCTION(BlueprintCallable)
	int GetArchitectureMode();

	UFUNCTION(BlueprintCallable)
	void SetArchitectureMode(int n);

	UFUNCTION(BlueprintCallable)
	void PlaceArchitecture();

	UFUNCTION(BlueprintCallable)
	void ClearArchitecture();

	UFUNCTION(BlueprintCallable)
	int AvailableArchBlocks();

	UFUNCTION(BlueprintCallable)
	int RequiredArchBlocks();

	// Events
	UPROPERTY(BlueprintAssignable, Category = "HUD")
	FOnPowerChanged PowerChanged;

	UPROPERTY(BlueprintAssignable, Category = "HUD")
	FOnCashChanged CashChanged;

	UPROPERTY(BlueprintAssignable, Category = "HUD")
	FOnDateChanged DateChanged;

	UPROPERTY(BlueprintAssignable, Category = "HUD")
	FOnPauseModeChanged PauseModeChanged;


private:

	const int WORLD_SCALE = 200;
	const float TEXTURE_SCALE = 5000.0f;
	void SetupNF();
	void SpawnChunks();
	void InitialModels();
	void UpdateModels();
	void AddModel(const nf::dynamic_model_t &model, TMap<int, ANoxStaticModel *> * container);
	void InitialLights();
	void UpdateLights();
	void AddLight(const nf::dynamic_lightsource_t &light);

	geometry_chunk water_geometry;
	void SetupWater();
	
};
