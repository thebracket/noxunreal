// Fill out your copyright notice in the Description page of Project Settings.

#include "NoxRegionChunk.h"

// Sets default values
ANoxRegionChunk::ANoxRegionChunk()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RegionLayers.SetNumUninitialized(nf::CHUNK_SIZE);
	RegionDesignLayers.SetNumUninitialized(nf::CHUNK_SIZE);
}

void ANoxRegionChunk::PostLoad() {
	Super::PostLoad();	
}

void ANoxRegionChunk::ChunkBuilder() {
	// Build the layer actors
	for (int i = 0; i < nf::CHUNK_SIZE; ++i) {
		// Create the layer
		FVector loc = FVector(0, 0, 0);
		FTransform trans = FTransform(loc);
		RegionLayers[i] = GetWorld()->SpawnActorDeferred<ANoxRegionLayer>(ANoxRegionLayer::StaticClass(), trans);
		RegionLayers[i]->chunk_idx = chunk_idx;
		RegionLayers[i]->base_x = base_x;
		RegionLayers[i]->base_y = base_y;
		RegionLayers[i]->base_z = base_z;
		RegionLayers[i]->local_z = i;
		RegionLayers[i]->FinishSpawning(trans);

		RegionDesignLayers[i] = GetWorld()->SpawnActorDeferred<ANoxRegionLayerDesign>(ANoxRegionLayerDesign::StaticClass(), trans);
		RegionDesignLayers[i]->chunk_idx = chunk_idx;
		RegionDesignLayers[i]->base_x = base_x;
		RegionDesignLayers[i]->base_y = base_y;
		RegionDesignLayers[i]->base_z = base_z;
		RegionDesignLayers[i]->local_z = i;
		RegionDesignLayers[i]->FinishSpawning(trans);
	}
	StaticModels();
	StaticFoliage();
}

void ANoxRegionChunk::Rebuild() {
	for (int i = 0; i < nf::CHUNK_SIZE; ++i) {
		RegionLayers[i]->Rebuild();
		RegionDesignLayers[i]->Rebuild();
	}
	StaticModels();
	StaticFoliage();
}

// Called when the game starts or when spawned
void ANoxRegionChunk::BeginPlay()
{
	Super::BeginPlay();
	ChunkBuilder();
	StaticModels();
	StaticFoliage();
}

void ANoxRegionChunk::StaticModels() 
{
	size_t size;
	nf::static_model_t * model_ptr;
	nf::chunk_models(chunk_idx, size, model_ptr);

	if (size > 0) {
		// We need to build some meshes! This needs to be made more efficent, we shouldn't delete everything...
		Models.Empty();
		for (size_t i = 0; i < size; ++i) {
			nf::static_model_t model = model_ptr[i];

			const float mx = model.x + 0.5f;
			const float my = model.y + 0.5f;
			const float mz = model.z;

			FVector loc = FVector(mx * WORLD_SCALE, my * WORLD_SCALE, mz * WORLD_SCALE);
			FTransform trans = FTransform(loc);
			auto newModel = GetWorld()->SpawnActorDeferred<ANoxStaticModel>(ANoxStaticModel::StaticClass(), trans);
			newModel->modelId = model.idx;
			newModel->x = model.x;
			newModel->y = model.y;
			newModel->z = model.z;
			newModel->FinishSpawning(trans);
			Models.Emplace(newModel);
		}
	}
}

// Called every frame
void ANoxRegionChunk::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ANoxRegionChunk::StaticFoliage() {
	size_t size;
	nf::veg_t * veg_ptr;
	nf::chunk_veg(chunk_idx, size, veg_ptr);
	for (int i = 0; i < nf::CHUNK_SIZE; ++i) {
		RegionLayers[i]->StaticFoliage(size, veg_ptr);
	}
}


