// (c) 2016 - Present, Bracket Productions

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Public/NoxGameInstance.h"
#include "ProceduralMeshComponent.h"
#include "Runtime/Engine/Classes/Components/HierarchicalInstancedStaticMeshComponent.h"
#include "NCharacter.h"
#include "Runtime/Engine/Classes/Components/PointLightComponent.h"
#include "NDisplayManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPauseChange, const int32, NewPauseMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCalendarChange, const FString, NewDateTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCompositeAdded, const int32, NewComposite);

struct GeometryChunk {
	TArray<FVector> vertices;
	TArray<int32> Triangles;
	TArray<FVector> normals;
	TArray<FVector2D> UV0;
	TArray<FProcMeshTangent> tangents;
	TArray<FLinearColor> vertexColors;
	int TriangleCounter = 0;

	void clear();
	void CreateFloor(int x, int y, int z, int w, int h);
	void CreateCube(int x, int y, int z, int w, int h, float d);
	void CreateWater(int x, int y, int z, int w, int h, float d);

	const int WORLD_SCALE = 200;
	const float TEXTURE_SCALE = 5000.0f;
};

USTRUCT(BlueprintType)
struct FFoliage
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere)
	UInstancedStaticMeshComponent * tree1 = nullptr;

	UPROPERTY(VisibleAnywhere)
	UInstancedStaticMeshComponent * tree2 = nullptr;

	UPROPERTY(VisibleAnywhere)
	UInstancedStaticMeshComponent * grass1 = nullptr;
};

USTRUCT(BlueprintType)
struct FNLayer
{
	GENERATED_USTRUCT_BODY()

	int base_x;
	int base_y;
	int base_z;
	int local_z;

	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent * mesh;

	UPROPERTY(VisibleAnywhere)
	FFoliage foliage;
};

USTRUCT(BlueprintType)
struct FNChunk
{
	GENERATED_USTRUCT_BODY()

	int base_x;
	int base_y;
	int base_z;
	int chunk_idx;
	TArray<FNLayer> layers;
};

UCLASS()
class NOXUNREAL_API ANDisplayManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANDisplayManager();

	USceneComponent * SceneComp;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Events
	UPROPERTY(BlueprintAssignable, Category = "HUD")
	FOnPauseChange OnPauseChange;

	UPROPERTY(BlueprintAssignable, Category = "HUD")
	FOnCalendarChange OnCalendarChange;

	UPROPERTY(BlueprintAssignable, Category = "Render")
	FCompositeAdded OnCompositeAdded;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Reference holders
	NRaws * raws;
	UBECS * ecs;
	UNPlanet * planet;
	UNRegion * region;
	
	// Materials
	UPROPERTY(BlueprintReadOnly)
	TMap<int, FString> MaterialAtlas;

	UPROPERTY(BlueprintReadOnly)
	TArray<FNChunk> Chunks;

	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent * WaterMesh;

	void RebuildChunkLayer(const int &chunk, const int &layer);
	FString GetMaterialTexture(const int &Key);

	UFUNCTION(BlueprintCallable)
	void onZChange();

	UPROPERTY(BlueprintReadOnly)
	TMap<int, UStaticMeshComponent *> Buildings;

	UPROPERTY(BlueprintReadOnly)
	TMap<int, ANCharacter *> CompositeRender;

	UPROPERTY(BlueprintReadOnly)
	TMap<int, UPointLightComponent *> Lightsources;

	void Water();
	void InitialBuildings();
	void InitialComposites();
	void InitialLights();

	FTimerHandle TickTockHandle;

	UFUNCTION(BlueprintCallable)
	void TickTock();
};
