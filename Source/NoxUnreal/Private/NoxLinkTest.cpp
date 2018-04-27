// Fill out your copyright notice in the Description page of Project Settings.

#include "NoxLinkTest.h"
#include "../../ThirdParty/libnox/Includes/libnox.h"

// Sets default values
ANoxLinkTest::ANoxLinkTest()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RegionChunks.SetNumUninitialized(nf::CHUNKS_TOTAL);

	mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));
	RootComponent = mesh;
	// New in UE 4.17, multi-threaded PhysX cooking.
	mesh->bUseAsyncCooking = true;
	mesh->bCastHiddenShadow = true;
}

void ANoxLinkTest::SetupNF() {
	FString ThePath = FPaths::ConvertRelativePathToFull(FPaths::GameContentDir());
	ThePath.Append("world_defs/");
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, nf::get_version());
	nf::setup_raws();
	nf::load_game();

	nf::chunks_init();
	nf::chunks_update();

	
}

void ANoxLinkTest::SpawnChunks() {
	if (GEngine) {		

		for (int idx = 0; idx < nf::CHUNKS_TOTAL; ++idx) {
			int x, y, z;
			nf::chunk_world_coordinates(idx, x, y, z);
			FVector loc = FVector(0, 0, 0);
			FTransform trans = FTransform(loc);
			FRotator rotate = FRotator(x * WORLD_SCALE, y * WORLD_SCALE, z * WORLD_SCALE);
			RegionChunks[idx] = GetWorld()->SpawnActorDeferred<ANoxRegionChunk>(ANoxRegionChunk::StaticClass(), trans);
			RegionChunks[idx]->chunk_idx = idx;
			RegionChunks[idx]->base_x = x;
			RegionChunks[idx]->base_y = y;
			RegionChunks[idx]->base_z = z;
			RegionChunks[idx]->FinishSpawning(trans);
		}

	}
}

void ANoxLinkTest::InitialModels()
{
	size_t size;
	nf::dynamic_model_t * model_ptr;
	nf::voxel_render_list(size, model_ptr);

	if (size > 0) {
		for (size_t i = 0; i < size; ++i) {
			nf::dynamic_model_t model = model_ptr[i];

			float mx = model.x + 0.5f;
			float my = model.y + 0.5f;
			const float mz = model.z;

			if (model.idx == 6) {
				mx += 1;
				my += 1;
			}

			FRotator rot = FRotator(model.axis1 * model.rot_angle, model.axis2 * model.rot_angle, model.axis3 * model.rot_angle);
			FVector loc = FVector(mx * WORLD_SCALE, my * WORLD_SCALE, mz * WORLD_SCALE);
			FTransform trans = FTransform(rot, loc);
			auto newModel = GetWorld()->SpawnActorDeferred<ANoxStaticModel>(ANoxStaticModel::StaticClass(), trans);
			newModel->modelId = model.idx;
			newModel->x = model.x;
			newModel->y = model.y;
			newModel->z = model.z;
			newModel->r = model.tint_r;
			newModel->g = model.tint_g;
			newModel->b = model.tint_b;
			newModel->FinishSpawning(trans);
			VoxModels.Emplace(newModel);
		}
	}
}

void ANoxLinkTest::InitialLights() {
	size_t size;
	nf::dynamic_lightsource_t * light_ptr;
	nf::lightsource_list(size, light_ptr);

	if (size > 0) {
		for (size_t i = 0; i < size; ++i) {
			nf::dynamic_lightsource_t light = light_ptr[i];
			FVector loc = FVector((light.x + 0.5f) * WORLD_SCALE, (light.y + 0.5f) * WORLD_SCALE, (light.z + 0.7f) * WORLD_SCALE);
			FTransform trans = FTransform(loc);
			auto Light = GetWorld()->SpawnActorDeferred<ANoxDynamicLight>(ANoxDynamicLight::StaticClass(), trans);
			Light->radius = light.radius;
			Light->r = light.r;
			Light->g = light.g;
			Light->b = light.b;
			Light->FinishSpawning(trans);
			DynamicLights.Emplace(Light);
		}
	}
}

void ANoxLinkTest::SetupWater() {
	size_t size;
	nf::water_t * water_ptr;
	nf::water_cubes(size, water_ptr);
	water_geometry.clear();
	mesh->ClearAllMeshSections();

	if (size > 0) {
		for (size_t i = 0; i < size; ++i) {
			nf::water_t water = water_ptr[i];
			water_geometry.CreateWater(water.x * WORLD_SCALE, water.y * WORLD_SCALE, water.z * WORLD_SCALE, 1 * WORLD_SCALE, 1 * WORLD_SCALE, water.depth * WORLD_SCALE);
		}
	}

	mesh->CreateMeshSection_LinearColor(0, water_geometry.vertices, water_geometry.Triangles, water_geometry.normals, water_geometry.UV0, water_geometry.vertexColors, water_geometry.tangents, true);
	UMaterial* material;
	material = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, TEXT("Material'/Game/Materials/12Water.12Water'"), nullptr, LOAD_None, nullptr));
	mesh->SetMaterial(0, material);
}

// Called when the game starts or when spawned
void ANoxLinkTest::BeginPlay()
{
	Super::BeginPlay();
	SetupNF();
	SpawnChunks();
	InitialModels();
	InitialLights();
	SetupWater();
}

void ANoxLinkTest::PostLoad() {
	Super::PostLoad();
	
}

void ANoxLinkTest::PostActorCreated() {
	Super::PostActorCreated();
}

// Called every frame
void ANoxLinkTest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update dirty chunks
	size_t dirty_size;
	int * dirty_ptr;

	nf::chunks_update_list_dirty(dirty_size, dirty_ptr);
	if (dirty_size > 0) {
		for (size_t i = 0; i < dirty_size; ++i) {
			int chunk_id = dirty_ptr[i];
			RegionChunks[chunk_id]->Rebuild();
		}
	}

	auto hudLink = nf::get_hud_info();
	PowerPercent = (float)hudLink.current_power / (float)hudLink.max_power;

	PowerDisplay = TEXT("Power: ");
	PowerDisplay.AppendInt(hudLink.current_power);
	PowerDisplay.Append(TEXT(" / "));
	PowerDisplay.AppendInt(hudLink.max_power);

	CashDisplay = TEXT("Cash: ");
	CashDisplay.AppendInt(hudLink.cash);
	CashDisplay.Append(TEXT(" Mcr"));

	DateDisplay = TEXT("");
	if (hudLink.month + 1 < 10) DateDisplay.Append(TEXT("0"));
	DateDisplay.AppendInt(hudLink.month+1);
	DateDisplay.Append(TEXT("/"));
	if (hudLink.day + 1 < 10) DateDisplay.Append(TEXT("0"));
	DateDisplay.AppendInt(hudLink.day+1);
	DateDisplay.Append(TEXT("/"));
	DateDisplay.AppendInt(hudLink.year);
	DateDisplay.Append(TEXT(" "));
	if (hudLink.hour < 10) DateDisplay.Append(TEXT("0"));
	DateDisplay.AppendInt(hudLink.hour);
	DateDisplay.Append(TEXT(":"));
	if (hudLink.minute < 10) DateDisplay.Append(TEXT("0"));
	DateDisplay.AppendInt(hudLink.minute);
	DateDisplay.Append(TEXT(":"));
	if (hudLink.second < 10) DateDisplay.Append(TEXT("0"));
	DateDisplay.AppendInt(hudLink.second);

	float hourAsFloat = (float)hudLink.hour + ((float)hudLink.minute / 60.0f);
	float TimeAsFloat = hourAsFloat / 24.0f; // Gets 0 - 1 range
	SunRotation = TimeAsFloat;

	auto pauseMode = nf::get_pause_mode();
	switch (pauseMode) {
	case 0: GameRunMode = TEXT("Running"); break;
	case 1: GameRunMode = TEXT("Paused"); break;
	case 2: GameRunMode = TEXT("Single Step"); break;
	}

	nf::on_tick(DeltaTime);
	if (nf::water_dirty) {
		SetupWater();
		nf::water_dirty = false; 
	}
}

void ANoxLinkTest::PauseGame() {
	nf::set_pause_mode(1);
}

void ANoxLinkTest::RunGame() {
	nf::set_pause_mode(0);
}

void ANoxLinkTest::SingleStepGame() {
	nf::set_pause_mode(2);
}