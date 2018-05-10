// Fill out your copyright notice in the Description page of Project Settings.

#include "NoxRegionChunk.h"

// Sets default values
ANoxRegionChunk::ANoxRegionChunk()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RegionLayers.SetNumUninitialized(nf::CHUNK_SIZE);
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
	}
	StaticModels();
	StaticFoliage();
}

void ANoxRegionChunk::Rebuild() {
	for (int i = 0; i < nf::CHUNK_SIZE; ++i) {
		RegionLayers[i]->Rebuild();
	}
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

void ANoxRegionChunk::FoliageInit(FString &voxAddress, UHierarchicalInstancedStaticMeshComponent *& target) {
	if (target == nullptr) {
		UStaticMesh* stairs;
		stairs = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *voxAddress, nullptr, LOAD_None, nullptr));

		target = NewObject<UHierarchicalInstancedStaticMeshComponent>(this);
		target->RegisterComponent();
		target->SetStaticMesh(stairs);
		//target->SetFlags(RF_Transactional);
		AddInstanceComponent(target);
	}
}

void ANoxRegionChunk::InitializeFoliageContainers() {
	FString grassPatch = FString("StaticMesh'/Game/Foliage/Meshes/Grass/SM_grass_patch1.SM_grass_patch1'");
	FString flowerPatch = FString("StaticMesh'/Game/Foliage/Meshes/Flowers/SM_grass_patch_flower1.SM_grass_patch_flower1'");
	FString bushPatch = FString("StaticMesh'/Game/Foliage/Meshes/Bush/SM_bush1.SM_bush1'");
	FString fatBush = FString("StaticMesh'/Game/Foliage/Meshes/Bush/SM_bush4.SM_bush4'");
	FString grassPatch2 = FString("StaticMesh'/Game/Foliage/Meshes/Grass/SM_grass_patch6.SM_grass_patch6'");

	FoliageInit(grassPatch, grass1);
	FoliageInit(flowerPatch, flower1);
	FoliageInit(bushPatch, bush1);
	FoliageInit(fatBush, bush2);
	FoliageInit(grassPatch2, grass2);
}

void ANoxRegionChunk::FoliageClear() {
	grass1->ClearInstances();
}

namespace impl {
	FRotator rot = FRotator();
	FVector grass_scale_normal = FVector(6.25f, 5.6f, 6.25f);
	FVector grass_scale_tall = FVector(6.25f, 5.6f, 10.0f);
	FVector grass_scale_small = FVector(6.25f, 5.6f, 3.0f);
	FVector grass_scale_tiny = FVector(6.25f, 5.6f, 1.0f);
}

void AddFoliageInstance(UHierarchicalInstancedStaticMeshComponent * target, FTransform &trans) {
	target->AddInstance(trans);
}

FVector GetFoliageScale(const int &plant, const int &lifecycle) {
	switch (plant) {
	case 0: return FVector(2.0f, 2.0f, 0.6f); // Artichoke
	case 1: return FVector(6.25f, 5.6f, 1.0f); // Asparagus
	case 2: return FVector(2.0f, 2.0f, 0.4f); // Bambara
	case 47: return FVector(2.0f, 2.0f, 1.0f); // Sage
	case 54: return FVector(6.25f, 5.6f, 1.0f); // Tomatillo
	case 55: return FVector(6.25f, 5.6f, 2.0f); // Tomato
	}

	switch (lifecycle) {
	case 0: return impl::grass_scale_tiny;
	case 1: return impl::grass_scale_small;
	case 2: return impl::grass_scale_normal;
	case 3: return impl::grass_scale_tall;
	}

	return impl::grass_scale_normal;
}

void ANoxRegionChunk::FoliageSieve(nf::veg_t &model) {
	using namespace impl;
	const float mx = model.x + 0.5f;
	const float my = model.y + 0.5f;
	const float mz = model.z;

	FVector loc = FVector(mx * 200, my * 200, mz * 200);

	UHierarchicalInstancedStaticMeshComponent * target;

	switch (model.plant) {
	case 0: target = bush2; break; // Artichoke
	case 2: target = bush2; break; // Artichoke
	case 17: target = flower1; break; // Daisy
	case 23: target = grass1; break;
	case 43: target = grass2; break; // Reeds
	case 47: target = bush1; break; // Sage
	case 54: target = flower1; break; // Tomatillo
	case 55: target = flower1; break; // Tomato
	default: target = grass1;
	}

	FTransform trans = FTransform(rot, loc, GetFoliageScale(model.plant, model.lifecycle));	

	AddFoliageInstance(target, trans);
}

void ANoxRegionChunk::StaticFoliage() 
{
	InitializeFoliageContainers();

	size_t size;
	nf::veg_t * veg_ptr;
	nf::chunk_veg(chunk_idx, size, veg_ptr);

	FoliageClear();

	if (size > 0) {
		for (size_t i = 0; i < size; ++i) {
			nf::veg_t model = veg_ptr[i];
			FoliageSieve(model);
		}
	}
}

