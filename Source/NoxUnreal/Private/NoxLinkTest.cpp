// Fill out your copyright notice in the Description page of Project Settings.

#include "NoxLinkTest.h"

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
	//ThePath.Append("world_defs/");
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, nf::get_version());
	nf::setup_raws();
	nf::setup_planet();
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

void ANoxLinkTest::AddModel(const nf::dynamic_model_t &model, TMap<int, ANoxStaticModel *> * container) {
	float mx = model.x + 0.5f;
	float my = model.y + 0.5f;
	const float mz = model.z;

	if (model.idx == 6) {
		mx += 1;
		my += 1;
	}

	FRotator rot = FRotator(model.axis1 * model.rot_angle, model.axis2 * model.rot_angle, model.axis3 * model.rot_angle + 90.0f);
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
	newModel->SetActorRotation(rot);
	container->Add(model.idx, newModel);
}

void ANoxLinkTest::InitialModels()
{
	size_t size;
	nf::dynamic_model_t * model_ptr;
	nf::voxel_render_list(size, model_ptr);

	if (size > 0) {
		for (size_t i = 0; i < size; ++i) {
			nf::dynamic_model_t model = model_ptr[i];

			if (!VoxModels.Contains(model.entity_id)) {
				VoxModels.Add(model.entity_id, TMap<int, ANoxStaticModel *>());
			}
			TMap<int, ANoxStaticModel *> * container = VoxModels.Find(model.entity_id);
			AddModel(model, container);			
		}
	}
}

void ANoxLinkTest::UpdateModels() {
	size_t size;
	nf::dynamic_model_t * model_ptr;
	nf::voxel_render_list(size, model_ptr);

	if (size == 0) {
		VoxModels.Empty();
	}
	else {
		for (size_t i = 0; i < size; ++i) {
			nf::dynamic_model_t model = model_ptr[i];

			if (!VoxModels.Contains(model.entity_id)) {
				// It's a brand new model entity!
				VoxModels.Add(model.entity_id, TMap<int, ANoxStaticModel *>());
				TMap<int, ANoxStaticModel *> * container = VoxModels.Find(model.entity_id);
				AddModel(model, container);
			}
			else {
				// It needs updating
				TMap<int, ANoxStaticModel *> * container = VoxModels.Find(model.entity_id);
				if (!container->Contains(model.idx)) {
					// It's a new subentry
					AddModel(model, container);
				}
				else {
					// Update an existing item
					ANoxStaticModel * target = *container->Find(model.idx);

					float mx = model.x + 0.5f;
					float my = model.y + 0.5f;
					const float mz = model.z;

					// Special case for large model
					if (model.idx == 6) {
						mx += 1;
						my += 1;
					}
					FRotator rot = FRotator(model.axis1 * model.rot_angle, model.axis2 * model.rot_angle, model.axis3 * model.rot_angle);
					FVector loc = FVector(mx * WORLD_SCALE, my * WORLD_SCALE, mz * WORLD_SCALE);
					FTransform trans = FTransform(rot, loc);

					target->modelId = model.idx;
					target->x = model.x;
					target->y = model.y;
					target->z = model.z;
					target->r = model.tint_r;
					target->g = model.tint_g;
					target->b = model.tint_b;
					target->SetActorTransform(trans);
				}
			}
		}
	}
}

void ANoxLinkTest::AddLight(const nf::dynamic_lightsource_t &light) {
	FVector loc = FVector((light.x + 0.5f) * WORLD_SCALE, (light.y + 0.5f) * WORLD_SCALE, (light.z + 0.7f) * WORLD_SCALE);
	FTransform trans = FTransform(loc);
	auto Light = GetWorld()->SpawnActorDeferred<ANoxDynamicLight>(ANoxDynamicLight::StaticClass(), trans);
	Light->radius = light.radius;
	Light->r = light.r;
	Light->g = light.g;
	Light->b = light.b;
	Light->FinishSpawning(trans);
	DynamicLights.Add(light.entity_id, Light);
}

void ANoxLinkTest::InitialLights() {
	size_t size;
	nf::dynamic_lightsource_t * light_ptr;
	nf::lightsource_list(size, light_ptr);

	if (size > 0) {
		for (size_t i = 0; i < size; ++i) {
			nf::dynamic_lightsource_t light = light_ptr[i];
			AddLight(light);
		}
	}
}

void ANoxLinkTest::UpdateLights() {
	size_t size;
	nf::dynamic_lightsource_t * light_ptr;
	nf::lightsource_list(size, light_ptr);

	if (size == 0) {
		DynamicLights.Empty();
	}
	else {
		for (size_t i = 0; i < size; ++i) {
			nf::dynamic_lightsource_t light = light_ptr[i];
			if (DynamicLights.Contains(light.entity_id)) {
				// Update the light
				ANoxDynamicLight * Light = *DynamicLights.Find(light.entity_id);
				Light->radius = light.radius;
				Light->r = light.r;
				Light->g = light.g;
				Light->b = light.b;
				Light->pointlight->SetLightColor(FLinearColor(light.r, light.g, light.b));
			}
			else {
				AddLight(light);
			}
		}

		// TODO: Delete stale lights
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
	nf::set_pause_mode(2); // Run one single step on start
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
	switch (hudLink.month) {
	case 0: DateDisplay.Append("JAN "); break;
	case 1: DateDisplay.Append("FEB "); break;
	case 2: DateDisplay.Append("MAR "); break;
	case 3: DateDisplay.Append("APR "); break;
	case 4: DateDisplay.Append("MAY "); break;
	case 5: DateDisplay.Append("JUN "); break;
	case 6: DateDisplay.Append("JUL "); break;
	case 7: DateDisplay.Append("AUG "); break;
	case 8: DateDisplay.Append("SEP "); break;
	case 9: DateDisplay.Append("OCT "); break;
	case 10: DateDisplay.Append("NOV "); break;
	case 11: DateDisplay.Append("DEC "); break;
	}
	if (hudLink.day + 1 < 10) DateDisplay.Append(TEXT("0"));
	DateDisplay.AppendInt(hudLink.day+1);
	DateDisplay.Append(TEXT(", "));
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
	UpdateModels();
	UpdateLights();
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


void ANoxLinkTest::GetUnitLists() {
	size_t sz;
	nf::unit_list_settler_t * sl;
	nf::get_unit_list_settlers(sz, sl);

	SettlerList.Empty();
	for (size_t i = 0; i < sz; ++i) {
		nf::unit_list_settler_t s = sl[i];

		FNoxSettlerListEntry n;
		n.name = FString(ANSI_TO_TCHAR(s.name));
		n.gender = FString(ANSI_TO_TCHAR(s.gender));
		n.profession = FString(ANSI_TO_TCHAR(s.profession));
		n.task = FString(ANSI_TO_TCHAR(s.task));
		n.hp = FString(ANSI_TO_TCHAR(s.hp));
		n.hp_percent = s.health_percent;
		n.id = s.id;

		SettlerList.Emplace(n);
	}

	nf::get_unit_list_natives(sz, sl);
	NativeList.Empty();
	for (size_t i = 0; i < sz; ++i) {
		nf::unit_list_settler_t s = sl[i];

		FNoxSettlerListEntry n;
		n.name = FString(ANSI_TO_TCHAR(s.name));
		n.gender = FString(ANSI_TO_TCHAR(s.gender));
		n.profession = FString(ANSI_TO_TCHAR(s.profession));
		n.task = FString(ANSI_TO_TCHAR(s.task));
		n.hp = FString(ANSI_TO_TCHAR(s.hp));
		n.hp_percent = s.health_percent;
		n.id = s.id;

		NativeList.Emplace(n);
	}

	nf::get_unit_list_wildlife(sz, sl);
	WildlifeList.Empty();
	for (size_t i = 0; i < sz; ++i) {
		nf::unit_list_settler_t s = sl[i];

		FNoxSettlerListEntry n;
		n.name = FString(ANSI_TO_TCHAR(s.name));
		n.gender = FString(ANSI_TO_TCHAR(s.gender));
		n.profession = FString(ANSI_TO_TCHAR(s.profession));
		n.task = FString(ANSI_TO_TCHAR(s.task));
		n.hp = FString(ANSI_TO_TCHAR(s.hp));
		n.hp_percent = s.health_percent;
		n.id = s.id;

		WildlifeList.Emplace(n);
	}
}

void ANoxLinkTest::ZoomSettler(int id) {
	nf::zoom_settler(id);
}

void ANoxLinkTest::FollowSettler(int id) {
	nf::follow_settler(id);
}

void ANoxLinkTest::SetWorldPositionFromMouse(FVector vec) {
	int x = vec.X / 200.0f;
	int y = vec.Y / 200.0f;
	int z = vec.Z / 200.0f;
	nf::set_world_pos_from_mouse(x, y, z);
}

void ANoxLinkTest::PopulateTooltip() {
	nf::tooltip_info_t tips = nf::get_tooltip_info();
	TooltipBlock.line1 = FString(ANSI_TO_TCHAR(tips.line1));
	TooltipBlock.line2 = FString(ANSI_TO_TCHAR(tips.line2));
	TooltipBlock.line3 = FString(ANSI_TO_TCHAR(tips.line3));
	TooltipBlock.line4 = FString(ANSI_TO_TCHAR(tips.line4));
	TooltipBlock.line5 = FString(ANSI_TO_TCHAR(tips.line5));
}

void ANoxLinkTest::UpdateSettlerJobList() {
	size_t sz;
	nf::settler_job_t * jl;

	nf::get_settler_job_list(sz, jl);

	SettlerJobList.Empty();
	for (size_t i = 0; i < sz; ++i) {
		nf::settler_job_t j = jl[i];

		FSettlerJob settler;
		settler.name = FString(ANSI_TO_TCHAR(j.name));
		settler.profession = FString(ANSI_TO_TCHAR(j.profession));
		settler.isMiner = j.is_miner;
		settler.isLumberjack = j.is_lumberjack;
		settler.isFarmer = j.is_farmer;
		settler.id = j.id;

		SettlerJobList.Emplace(settler);
	}
}

void ANoxLinkTest::MakeFarmer(int id) {
	nf::make_farmer(id);
}

void ANoxLinkTest::MakeLumberjack(int id) {
	nf::make_lumberjack(id);
}

void ANoxLinkTest::MakeMiner(int id) {
	nf::make_miner(id);
}

void ANoxLinkTest::FireFarmer(int id) {
	nf::fire_farmer(id);
}

void ANoxLinkTest::FireLumberjack(int id) {
	nf::fire_lumberjack(id);
}

void ANoxLinkTest::FireMiner(int id) {
	nf::fire_miner(id);
}