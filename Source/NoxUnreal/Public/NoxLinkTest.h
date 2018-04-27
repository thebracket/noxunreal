// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NoxRegionChunk.h"
#include "NoxDynamicLight.h"
#include "ProceduralMeshComponent.h"
#include "NoxLinkTest.generated.h"

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
	TArray<ANoxStaticModel *> VoxModels;
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

private:

	const int WORLD_SCALE = 200;
	const float TEXTURE_SCALE = 5000.0f;
	void SetupNF();
	void SpawnChunks();
	void InitialModels();
	void InitialLights();

	geometry_chunk water_geometry;
	void SetupWater();
	
};
