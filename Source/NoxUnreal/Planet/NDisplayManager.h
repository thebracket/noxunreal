// (c) 2016 - Present, Bracket Productions

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Public/NoxGameInstance.h"
#include "ProceduralMeshComponent.h"
#include "NDisplayManager.generated.h"

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
struct FNLayer
{
	GENERATED_USTRUCT_BODY()

	int base_x;
	int base_y;
	int base_z;
	int local_z;

	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent * mesh;

	//TMap<int, GeometryChunk> geometry_by_material;
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

	void RebuildChunkLayer(const int &chunk, const int &layer);
	FString GetMaterialTexture(const int &Key);
};
