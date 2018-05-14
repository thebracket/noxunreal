// (c) 2016 - Present, Bracket Productions

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NoxRegionLayer.h"
#include "NoxRegionLayerDesign.generated.h"

UCLASS()
class NOXUNREAL_API ANoxRegionLayerDesign : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANoxRegionLayerDesign();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent * mesh;

	TMap<int, geometry_chunk> geometry_by_material;

	const int WORLD_SCALE = 200;
	const float TEXTURE_SCALE = 2000.0f;
	int base_x;
	int base_y;
	int base_z;
	int local_z;
	int chunk_idx;
	
	void ChunkBuilder();
	void Rebuild();
	
};
