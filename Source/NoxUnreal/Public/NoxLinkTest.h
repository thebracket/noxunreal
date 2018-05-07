// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NoxRegionChunk.h"
#include "NoxDynamicLight.h"
#include "ProceduralMeshComponent.h"
#include "../../ThirdParty/libnox/Includes/libnox.h"
#include "NoxLinkTest.generated.h"

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

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	TArray<ANoxRegionChunk *> RegionChunks;
	TMap<int, TMap<int, ANoxStaticModel *>> VoxModels;
	TMap<int, ANoxDynamicLight *> DynamicLights;

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
