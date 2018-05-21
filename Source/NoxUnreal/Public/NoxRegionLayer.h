// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "Runtime/Engine/Classes/Components/HierarchicalInstancedStaticMeshComponent.h"
#include "../../ThirdParty/libnox/Includes/libnox.h"
#include "../../ThirdParty/libnox/Includes/noxconsts.h"
#include "NoxRegionLayer.generated.h"

struct geometry_chunk {
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

UCLASS()
class NOXUNREAL_API ANoxRegionLayer : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANoxRegionLayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent * mesh;

	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent * designMesh;

	TMap<int, geometry_chunk> geometry_by_material;
	TMap<int, geometry_chunk> design_geometry_by_material;

	const int WORLD_SCALE = 200;
	const float TEXTURE_SCALE = 2000.0f;
	int base_x;
	int base_y;
	int base_z;
	int local_z;
	int chunk_idx;

	void ChunkBuilder();
	void Rebuild();
	
	// Foliage
	UHierarchicalInstancedStaticMeshComponent * grass1 = nullptr;
	UHierarchicalInstancedStaticMeshComponent * grass2 = nullptr;
	UHierarchicalInstancedStaticMeshComponent * flower1 = nullptr;
	UHierarchicalInstancedStaticMeshComponent * bush1 = nullptr;
	UHierarchicalInstancedStaticMeshComponent * bush2 = nullptr;
	UHierarchicalInstancedStaticMeshComponent * tree1 = nullptr;
	UHierarchicalInstancedStaticMeshComponent * tree2 = nullptr;
	void InitializeFoliageContainers();
	void FoliageInit(FString &voxAddress, UHierarchicalInstancedStaticMeshComponent *& target);
	void FoliageClear();
	void FoliageVisibility(const bool &vis);
	void FoliageSieve(nf::veg_t &model);
	void StaticFoliage(size_t &size, nf::veg_t *& veg_ptr);

	UFUNCTION()
	void onZChange();
};
