// (c) 2016 - Present, Bracket Productions

#include "NDisplayManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameInputManager.h"

constexpr float WORLD_SCALE = 200.0f;

// Sets default values
ANDisplayManager::ANDisplayManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultScene"));
	RootComponent = SceneComp;
}

FRotator rot = FRotator(0.0f, 0.0f, 0.0f);
FVector GrassScale(6.25f, 6.25f, 6.25f);
FVector TreeScale(30.0f, 30.0f, 30.0f);

// Called when the game starts or when spawned
void ANDisplayManager::BeginPlay()
{
	Super::BeginPlay();
	
	// Link references
	UNoxGameInstance * game = Cast<UNoxGameInstance>(UGameplayStatics::GetGameInstance(this));
	raws = game->GetRaws();
	planet = game->GetPlanet();
	region = game->GetRegion();
	ecs = game->GetECS();
	ecs->LinkDM(this);

	// Setup Materials
	MaterialAtlas.Empty();
	for (const auto &ma : raws->texture_atlas) {
		const int key = ma.Key;
		const FString value = ma.Value;
		MaterialAtlas.Add(key, value);
	}
	MaterialAtlas.Add(-1, TEXT("MaterialInstanceConstant'/Game/TileMaterials/Instances/MZ_Grass.MZ_Grass'"));
	MaterialAtlas.Add(-2, TEXT("MaterialInstanceConstant'/Game/TileMaterials/Instances/MZ_Plastic.MZ_Plastic'"));
	MaterialAtlas.Add(-3, TEXT("MaterialInstanceConstant'/Game/TileMaterials/Instances/MB_Window.MB_Window'"));

	// Init/Setup Chunks
	region->InitializeChunks();
	region->UpdateChunks();

	// Spawn Chunks
	Chunks.Empty();
	for (size_t i = 0; i < nfu::CHUNKS_TOTAL; ++i) {
		FNChunk chunk;
		chunk.base_x = region->Chunks[i].base_x;
		chunk.base_y = region->Chunks[i].base_y;
		chunk.base_z = region->Chunks[i].base_z;
		chunk.chunk_idx = region->Chunks[i].index;
		chunk.layers.Empty();
		for (int j = 0; j < nfu::CHUNK_SIZE; ++j) {
			FNLayer layer;
			layer.base_x = chunk.base_x;
			layer.base_y = chunk.base_y;
			layer.base_z = chunk.base_z;
			layer.local_z = j;
			chunk.layers.Emplace(layer);
		}

		Chunks.Emplace(chunk);
	}

	UStaticMesh* tree1 = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, TEXT("StaticMesh'/Game/TileMaterials/Foliage/FastTree/tree_1__tree.tree_1__tree'"), nullptr, LOAD_None, nullptr));
	UStaticMesh* tree2 = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, TEXT("StaticMesh'/Game/TileMaterials/Foliage/FastTree/tree_1__leaves.tree_1__leaves'"), nullptr, LOAD_None, nullptr));
	UStaticMesh* grass = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, TEXT("StaticMesh'/Game/Meshes/Grass/SM_grass_patch1.SM_grass_patch1'"), nullptr, LOAD_None, nullptr));

	for (size_t i = 0; i < nfu::CHUNKS_TOTAL; ++i) {
		for (size_t j = 0; j < nfu::CHUNK_SIZE; ++j) {
			FString layerName = "Mesh";
			layerName.AppendInt(i);
			layerName.Append("L");
			layerName.AppendInt(j);
			Chunks[i].layers[j].mesh = NewObject<UProceduralMeshComponent>(this, FName(*layerName));
			Chunks[i].layers[j].mesh->RegisterComponent();
			//chunks[i].layers[j].mesh->AttachTo(GetRootComponent(), SocketName, EAttachLocation::KeepWorldPosition);
			RebuildChunkLayer(i, j);			
		}

		for (const auto &Foliage : region->Chunks[i].vegetation_models) {
			// plant, state, x, y, z - tuple contents
			const int plant = Foliage.Get<0>();
			const int state = Foliage.Get<1>();
			const int x = Foliage.Get<2>();
			const int y = Foliage.Get<3>();
			const int z = Foliage.Get<4>();
			float mx = x + 0.5f;
			float my = y + 0.5f;
			float mz = z;

			const int plant_layer = z - Chunks[i].base_z;

			if (Chunks[i].layers[plant_layer].foliage.tree1 == nullptr) {
				// Initialize foliage containers
				Chunks[i].layers[plant_layer].foliage.tree1 = NewObject<UInstancedStaticMeshComponent>(this);
				Chunks[i].layers[plant_layer].foliage.tree1->RegisterComponent();
				Chunks[i].layers[plant_layer].foliage.tree1->SetStaticMesh(tree1);
				Chunks[i].layers[plant_layer].foliage.tree1->bOwnerNoSee = false;
				Chunks[i].layers[plant_layer].foliage.tree1->bCastDynamicShadow = false;
				Chunks[i].layers[plant_layer].foliage.tree1->CastShadow = false;
				Chunks[i].layers[plant_layer].foliage.tree1->SetHiddenInGame(false);
				Chunks[i].layers[plant_layer].foliage.tree1->SetMobility(EComponentMobility::Movable);
				Chunks[i].layers[plant_layer].foliage.tree1->AttachTo(RootComponent);
				AddInstanceComponent(Chunks[i].layers[plant_layer].foliage.tree1);

				Chunks[i].layers[plant_layer].foliage.tree2 = NewObject<UInstancedStaticMeshComponent>(this);
				Chunks[i].layers[plant_layer].foliage.tree2->RegisterComponent();
				Chunks[i].layers[plant_layer].foliage.tree2->bOwnerNoSee = false;
				Chunks[i].layers[plant_layer].foliage.tree2->bCastDynamicShadow = false;
				Chunks[i].layers[plant_layer].foliage.tree2->CastShadow = false;
				Chunks[i].layers[plant_layer].foliage.tree2->SetHiddenInGame(false);
				Chunks[i].layers[plant_layer].foliage.tree2->SetStaticMesh(tree2);
				AddInstanceComponent(Chunks[i].layers[plant_layer].foliage.tree2);

				Chunks[i].layers[plant_layer].foliage.grass1 = NewObject<UInstancedStaticMeshComponent>(this);
				Chunks[i].layers[plant_layer].foliage.grass1->RegisterComponent();
				Chunks[i].layers[plant_layer].foliage.grass1->bOwnerNoSee = false;
				Chunks[i].layers[plant_layer].foliage.grass1->bCastDynamicShadow = false;
				Chunks[i].layers[plant_layer].foliage.grass1->CastShadow = false;
				Chunks[i].layers[plant_layer].foliage.grass1->SetHiddenInGame(false);
				Chunks[i].layers[plant_layer].foliage.grass1->SetStaticMesh(grass);
				AddInstanceComponent(Chunks[i].layers[plant_layer].foliage.grass1);
			}

			FVector loc = FVector(mx * WORLD_SCALE, my * WORLD_SCALE, mz * WORLD_SCALE);

			if (plant == -1) {
				FTransform trans = FTransform(rot, loc, TreeScale);
				Chunks[i].layers[plant_layer].foliage.tree1->AddInstance(trans);
				Chunks[i].layers[plant_layer].foliage.tree2->AddInstance(trans);
			}
			else {
				FTransform trans = FTransform(rot, loc, GrassScale);
				Chunks[i].layers[plant_layer].foliage.grass1->AddInstance(trans);
			}
		}
	}

	// Water
	WaterMesh = NewObject<UProceduralMeshComponent>(this, FName(TEXT("WetMesh")));
	WaterMesh->RegisterComponent();
	Water();
	InitialBuildings();
	InitialComposites();
	InitialLights();

	// Link to event handlers
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGameInputManager::StaticClass(), FoundActors);
	for (int i = 0; i < FoundActors.Num(); ++i) {
		AGameInputManager * cd = Cast<AGameInputManager>(FoundActors[i]);
		//cd->ZLevelChanged.Broadcast();
		cd->ZLevelChanged.AddDynamic(this, &ANDisplayManager::onZChange);
	}
	OnCompositeMoved.AddDynamic(this, &ANDisplayManager::onCompositeMove);
	OnSettlerEmote.AddDynamic(this, &ANDisplayManager::onEmote);

	// Single step and start the timer
	ecs->SetPauseMode(1);
	GetWorld()->GetTimerManager().SetTimer(TickTockHandle, this, &ANDisplayManager::TickTock, 0.066F, true, 0.066F);
}

// Called every frame
void ANDisplayManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ANDisplayManager::RebuildChunkLayer(const int &chunk, const int &layer) {
	TMap<int, GeometryChunk> geometry_by_material;
	auto * gbm = &geometry_by_material;
	//auto * gbm = &Chunks[chunk].layers[layer].geometry_by_material;
	gbm->Empty();

	int z = Chunks[chunk].layers[layer].local_z;
	for (const auto &floor : region->Chunks[chunk].layers[layer].floors) {
		GeometryChunk * g = nullptr;
		if (!gbm->Contains(floor.tex)) {
			gbm->Add(floor.tex, GeometryChunk());
		}
		g = gbm->Find(floor.tex);

		//geometry.CreateFloor(floor.x * WORLD_SCALE, floor.y * WORLD_SCALE, floor.z * WORLD_SCALE, floor.w * WORLD_SCALE, floor.h * WORLD_SCALE);
		g->CreateFloor(floor.x * WORLD_SCALE, floor.y * WORLD_SCALE, floor.z * WORLD_SCALE, floor.w * WORLD_SCALE, floor.h * WORLD_SCALE);
	}

	for (const auto &cube : region->Chunks[chunk].layers[layer].cubes) {
		GeometryChunk * g = nullptr;
		if (!gbm->Contains(cube.tex)) {
			gbm->Add(cube.tex, GeometryChunk());
		}
		g = gbm->Find(cube.tex);

		//geometry.CreateCube(cube.x * WORLD_SCALE, cube.y * WORLD_SCALE, cube.z * WORLD_SCALE, cube.w * WORLD_SCALE, cube.h * WORLD_SCALE, cube.d * WORLD_SCALE);
		g->CreateCube(cube.x * WORLD_SCALE, cube.y * WORLD_SCALE, cube.z * WORLD_SCALE, cube.w * WORLD_SCALE, cube.h * WORLD_SCALE, cube.d * WORLD_SCALE);
	}

	int sectionCount = 0;
	for (auto &gm : *gbm) {
		FString MaterialAddress = GetMaterialTexture(gm.Key);

		UMaterialInstance* material;
		material = Cast<UMaterialInstance>(StaticLoadObject(UMaterialInstance::StaticClass(), nullptr, *MaterialAddress, nullptr, LOAD_None, nullptr));
		Chunks[chunk].layers[layer].mesh->SetMaterial(sectionCount, material);

		Chunks[chunk].layers[layer].mesh->CreateMeshSection_LinearColor(sectionCount, gm.Value.vertices, gm.Value.Triangles, gm.Value.normals, gm.Value.UV0, gm.Value.vertexColors, gm.Value.tangents, true);
		++sectionCount;
	}

	// Enable collision data
	Chunks[chunk].layers[layer].mesh->ContainsPhysicsTriMeshData(true);
	Chunks[chunk].layers[layer].mesh->bCastHiddenShadow = true;
	region->Chunks[chunk].layers[layer].floors.Empty();
	region->Chunks[chunk].layers[layer].cubes.Empty();
}

FString ANDisplayManager::GetMaterialTexture(const int &Key) {

	if (MaterialAtlas.Contains(Key) && MaterialAtlas[Key].Len() > 0) {
		return MaterialAtlas[Key];
	}
	else {
		return MaterialAtlas[-2];
	}
}

void ANDisplayManager::onZChange() {
	for (int chunk = 0; chunk < nfu::CHUNKS_TOTAL; ++chunk) {
		for (int layer = 0; layer < nfu::CHUNK_SIZE; ++layer) {
			const int z = ecs->CameraPosition->region_z;
			const int my_layer = Chunks[chunk].base_z + layer;

			if (my_layer <= z && my_layer >= z - 20) {
				Chunks[chunk].layers[layer].mesh->SetVisibility(true);
				Chunks[chunk].layers[layer].mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
				if (Chunks[chunk].layers[layer].foliage.tree1) {
					Chunks[chunk].layers[layer].foliage.tree1->SetVisibility(true);
					Chunks[chunk].layers[layer].foliage.tree2->SetVisibility(true);
					Chunks[chunk].layers[layer].foliage.grass1->SetVisibility(true);
				}
			}
			else {
				Chunks[chunk].layers[layer].mesh->SetVisibility(false);
				Chunks[chunk].layers[layer].mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
				if (Chunks[chunk].layers[layer].foliage.tree1) {
					Chunks[chunk].layers[layer].foliage.tree1->SetVisibility(false);
					Chunks[chunk].layers[layer].foliage.tree2->SetVisibility(false);
					Chunks[chunk].layers[layer].foliage.grass1->SetVisibility(false);
				}
			}
		}
	}

	const int camera_z = ecs->CameraPosition->region_z;
	for (auto &bm : Buildings) {
		auto loc = bm.Value->RelativeLocation;
		int z = (loc.Z-0.5f) / WORLD_SCALE;
		
		if (camera_z > z) {
			bm.Value->SetVisibility(true);
		}
		else {
			bm.Value->SetVisibility(false);
		}
	}

	for (auto &cr : CompositeRender) {
		const int z = cr.Value->z;
		if (camera_z >= z) {
			cr.Value->Show(true);
		}
		else {
			cr.Value->Show(false);
		}
	}
}

void ANDisplayManager::Water() {
	using namespace nfu;

	WaterMesh->ClearAllMeshSections();
	GeometryChunk water_geometry;

	for (int z = 0; z < REGION_DEPTH; ++z) {
		for (int y = 0; y < REGION_HEIGHT; ++y) {
			for (int x = 0; x < REGION_WIDTH; ++x) {
				const auto idx = region->mapidx(x, y, z);
				if (region->WaterLevel[idx] > 0) {
					water_geometry.CreateWater(x * WORLD_SCALE, y * WORLD_SCALE, z * WORLD_SCALE, 1 * WORLD_SCALE, 1 * WORLD_SCALE, region->WaterLevel[idx]);
				}
			}
		}
	}

	WaterMesh->CreateMeshSection_LinearColor(0, water_geometry.vertices, water_geometry.Triangles, water_geometry.normals, water_geometry.UV0, water_geometry.vertexColors, water_geometry.tangents, true);
	UMaterial* material;
	material = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, TEXT("Material'/Game/Materials/12Water.12Water'"), nullptr, LOAD_None, nullptr));
	WaterMesh->SetMaterial(0, material);
}

FString GetModelPath(const int &modelId) {
	FString voxAddress;
	switch (modelId) {
	case -1: voxAddress = "StaticMesh'/Game/Foliage/Meshes/Trees/SM_tree1.SM_tree1'"; break;
	case 1: voxAddress = "StaticMesh'/Game/Models/cryobed32.cryobed32'"; break;
	case 2: voxAddress = "StaticMesh'/Game/Models/floorlight.floorlight'"; break;
	case 3: voxAddress = "StaticMesh'/Game/Models/rtg.rtg'"; break;
	case 4: voxAddress = "StaticMesh'/Game/Models/cabinet.cabinet'"; break;
	case 5: voxAddress = "StaticMesh'/Game/Models/battery.battery'"; break;
	case 6: voxAddress = "StaticMesh'/Game/Models/cordex.cordex'"; break;
	case 7: voxAddress = "StaticMesh'/Game/Models/replicator-small.replicator-small'"; break;
	case 8: voxAddress = "StaticMesh'/Game/Models/solarpanel.solarpanel'"; break;
	case 9: voxAddress = "StaticMesh'/Game/Models/woodlogs.woodlogs'"; break;
	case 10: voxAddress = "StaticMesh'/Game/Models/turret.turret'"; break;
	case 11: voxAddress = "StaticMesh'/Game/Models/simpleaxe.simpleaxe'"; break;
	case 12: voxAddress = "StaticMesh'/Game/Models/crate-fakefire.crate-fakefire'"; break;
	case 13: voxAddress = "StaticMesh'/Game/Models/crate-tent.crate-tent'"; break;
	case 14: voxAddress = "StaticMesh'/Game/Models/campfire-fake.campfire-fake'"; break;
	case 15: voxAddress = "StaticMesh'/Game/Models/tent.tent'"; break;
	case 16: voxAddress = "StaticMesh'/Game/Models/sawmill.sawmill'"; break;
	case 17: voxAddress = "StaticMesh'/Game/Models/block.block'"; break;
	case 18: voxAddress = "StaticMesh'/Game/Models/wastewood.wastewood'"; break;
	case 19: voxAddress = "StaticMesh'/Game/Models/bonecarver.bonecarver'"; break;
	case 20: voxAddress = "StaticMesh'/Game/Models/butcher.butcher'"; break;
	case 21: voxAddress = "StaticMesh'/Game/Models/carpenter.carpenter'"; break;
	case 22: voxAddress = "StaticMesh'/Game/Models/chemist.chemist'"; break;
	case 23: voxAddress = "StaticMesh'/Game/Models/stairs-up.stairs-up'"; break;
	case 24: voxAddress = "StaticMesh'/Game/Models/stairs-down.stairs-down'"; break;
	case 25: voxAddress = "StaticMesh'/Game/Models/stairs-updown.stairs-updown'"; break;
	case 26: voxAddress = "StaticMesh'/Game/Models/tailor.tailor'"; break;
	case 27: voxAddress = "StaticMesh'/Game/Models/forge.forge'"; break;
	case 28: voxAddress = "StaticMesh'/Game/Models/simplepick.simplepick'"; break;
	case 29: voxAddress = "StaticMesh'/Game/Models/boulder.boulder'"; break;
	case 30: voxAddress = "StaticMesh'/Game/Models/ore.ore'"; break;
	case 31: voxAddress = "StaticMesh'/Game/Models/powder.powder'"; break;
	case 32: voxAddress = "StaticMesh'/Game/Models/oil.oil'"; break;
	case 33: voxAddress = "StaticMesh'/Game/Models/stonecutter.stonecutter'"; break;
	case 34: voxAddress = "StaticMesh'/Game/Models/smelter.smelter'"; break;
	case 35: voxAddress = "StaticMesh'/Game/Models/deer.deer'"; break;
	case 36: voxAddress = "StaticMesh'/Game/Models/horse.horse'"; break;
	case 37: voxAddress = "StaticMesh'/Game/Models/armadillo.armadillo'"; break;
	case 38: voxAddress = "StaticMesh'/Game/Models/badger.badger'"; break;
	case 39: voxAddress = "StaticMesh'/Game/Models/badger.badger'"; break;
	case 40: voxAddress = "StaticMesh'/Game/Models/charcoal_hut.charcoal_hut'"; break;
	case 41: voxAddress = "StaticMesh'/Game/Models/still.still'"; break;
	case 42: voxAddress = "StaticMesh'/Game/Models/loom.loom'"; break;
	case 43: voxAddress = "StaticMesh'/Game/Models/glass_furnace.glass_furnace'"; break;
	case 44: voxAddress = "StaticMesh'/Game/Models/mason.mason'"; break;
	case 45: voxAddress = "StaticMesh'/Game/Models/mechanic.mechanic'"; break;
	case 46: voxAddress = "StaticMesh'/Game/Models/tanner.tanner'"; break;
	case 47: voxAddress = "StaticMesh'/Game/Models/charcoal.charcoal'"; break;
	case 48: voxAddress = "StaticMesh'/Game/Models/ant.ant'"; break;
	case 49: voxAddress = "StaticMesh'/Game/Models/person_base.person_base'"; break;
	case 50: voxAddress = "StaticMesh'/Game/Models/person_hair_short.person_hair_short'"; break;
	case 51: voxAddress = "StaticMesh'/Game/Models/person_hair_long.person_hair_long'"; break;
	case 52: voxAddress = "StaticMesh'/Game/Models/person_hair_pigtails.person_hair_pigtails'"; break;
	case 53: voxAddress = "StaticMesh'/Game/Models/person_hair_mohawk.person_hair_mohawk'"; break;
	case 54: voxAddress = "StaticMesh'/Game/Models/person_hair_balding.person_hair_balding'"; break;
	case 55: voxAddress = "StaticMesh'/Game/Models/person_hair_triangle.person_hair_triangle'"; break;
	case 56: voxAddress = "StaticMesh'/Game/Models/clothes_batman_hat.clothes_batman_hat'"; break;
	case 57: voxAddress = "StaticMesh'/Game/Models/clothes_police_hat.clothes_police_hat'"; break;
	case 58: voxAddress = "StaticMesh'/Game/Models/clothes_cat_ears.clothes_cat_ears'"; break;
	case 59: voxAddress = "StaticMesh'/Game/Models/clothes_tiara.clothes_tiara'"; break;
	case 60: voxAddress = "StaticMesh'/Game/Models/clothes_formal_shirt.clothes_formal_shirt'"; break;
	case 61: voxAddress = "StaticMesh'/Game/Models/clothes_bathrobe.clothes_bathrobe'"; break;
	case 62: voxAddress = "StaticMesh'/Game/Models/clothes_spandex_shirt.clothes_spandex_shirt'"; break;
	case 63: voxAddress = "StaticMesh'/Game/Models/clothes_sports_shirt.clothes_sports_shirt'"; break;
	case 64: voxAddress = "StaticMesh'/Game/Models/clothes_tshirt.clothes_tshirt'"; break;
	case 65: voxAddress = "StaticMesh'/Game/Models/clothes_cargopants.clothes_cargopants'"; break;
	case 66: voxAddress = "StaticMesh'/Game/Models/clothes_longskirt.clothes_longskirt'"; break;
	case 67: voxAddress = "StaticMesh'/Game/Models/clothes_hawaiian_shorts.clothes_hawaiian_shorts'"; break;
	case 68: voxAddress = "StaticMesh'/Game/Models/clothes_spandex_pants.clothes_spandex_pants'"; break;
	case 69: voxAddress = "StaticMesh'/Game/Models/clothes_miniskirt.clothes_miniskirt'"; break;
	case 70: voxAddress = "StaticMesh'/Game/Models/clothes_shoes.clothes_shoes'"; break;
	case 71: voxAddress = "StaticMesh'/Game/Models/clothes_boots.clothes_boots'"; break;
	case 72: voxAddress = "StaticMesh'/Game/Models/clothes_simple_tunic.clothes_simple_tunic'"; break;
	case 73: voxAddress = "StaticMesh'/Game/Models/clothes_britches_simple.clothes_britches_simple'"; break;
	case 74: voxAddress = "StaticMesh'/Game/Models/clothes_cap_simple.clothes_cap_simple'"; break;
	case 75: voxAddress = "StaticMesh'/Game/Models/ground_helmet.ground_helmet'"; break;
	case 76: voxAddress = "StaticMesh'/Game/Models/ground_shirt.ground_shirt'"; break;
	case 77: voxAddress = "StaticMesh'/Game/Models/ground_pants.ground_pants'"; break;
	case 78: voxAddress = "StaticMesh'/Game/Models/ground_shoes.ground_shoes'"; break;
	case 79: voxAddress = "StaticMesh'/Game/Models/meat.meat'"; break;
	case 80: voxAddress = "StaticMesh'/Game/Models/foodplate.foodplate'"; break;
	case 81: voxAddress = "StaticMesh'/Game/Models/liquid.liquid'"; break;
	case 82: voxAddress = "StaticMesh'/Game/Models/veggie.veggie'"; break;
	case 83: voxAddress = "StaticMesh'/Game/Models/table.table'"; break;
	case 84: voxAddress = "StaticMesh'/Game/Models/chair.chair'"; break;
	case 85: voxAddress = "StaticMesh'/Game/Models/door_ground.door_ground'"; break;
	case 86: voxAddress = "StaticMesh'/Game/Models/bed.bed'"; break;
	case 87: voxAddress = "StaticMesh'/Game/Models/axeblade.axeblade'"; break;
	case 88: voxAddress = "StaticMesh'/Game/Models/lathe.lathe'"; break;
	case 89: voxAddress = "StaticMesh'/Game/Models/hay.hay'"; break;
	case 90: voxAddress = "StaticMesh'/Game/Models/thread.thread'"; break;
	case 91: voxAddress = "StaticMesh'/Game/Models/cloth.cloth'"; break;
	case 92: voxAddress = "StaticMesh'/Game/Models/paper.paper'"; break;
	case 93: voxAddress = "StaticMesh'/Game/Models/dung.dung'"; break;
	case 94: voxAddress = "StaticMesh'/Game/Models/mechanism.mechanism'"; break;
	case 95: voxAddress = "StaticMesh'/Game/Models/circuit.circuit'"; break;
	case 96: voxAddress = "StaticMesh'/Game/Models/bone.bone'"; break;
	case 97: voxAddress = "StaticMesh'/Game/Models/hide.hide'"; break;
	case 98: voxAddress = "StaticMesh'/Game/Models/skull.skull'"; break;
	case 99: voxAddress = "StaticMesh'/Game/Models/cage.cage'"; break;
	case 100: voxAddress = "StaticMesh'/Game/Models/club.club'"; break;
	case 101: voxAddress = "StaticMesh'/Game/Models/club_spiked.club_spiked'"; break;
	case 102: voxAddress = "StaticMesh'/Game/Models/pointystick.pointystick'"; break;
	case 103: voxAddress = "StaticMesh'/Game/Models/hammer.hammer'"; break;
	case 104: voxAddress = "StaticMesh'/Game/Models/sword.sword'"; break;
	case 105: voxAddress = "StaticMesh'/Game/Models/knife.knife'"; break;
	case 106: voxAddress = "StaticMesh'/Game/Models/atlatl.atlatl'"; break;
	case 107: voxAddress = "StaticMesh'/Game/Models/bow.bow'"; break;
	case 108: voxAddress = "StaticMesh'/Game/Models/arrow.arrow'"; break;
	case 109: voxAddress = "StaticMesh'/Game/Models/crossbow.crossbow'"; break;
	case 110: voxAddress = "StaticMesh'/Game/Models/corpse.corpse'"; break;
	case 111: voxAddress = "StaticMesh'/Game/Models/simplepick_held.simplepick_held'"; break;
	case 112: voxAddress = "StaticMesh'/Game/Models/simpleaxe_held.simpleaxe_held'"; break;
	case 113: voxAddress = "StaticMesh'/Game/Models/hoe.hoe'"; break;
	case 114: voxAddress = "StaticMesh'/Game/Models/dung_heap.dung_heap'"; break;
	case 115: voxAddress = "StaticMesh'/Game/Models/seed.seed'"; break;
	case 116: voxAddress = "StaticMesh'/Game/Models/raised_flower_bed.raised_flower_bed'"; break;
	case 117: voxAddress = "StaticMesh'/Game/Models/wall.wall'"; break;
	case 118: voxAddress = "StaticMesh'/Game/Models/floor.floor'"; break;
	case 119: voxAddress = "StaticMesh'/Game/Models/ramp.ramp'"; break;
	case 120: voxAddress = "StaticMesh'/Game/Models/carpenter_intermediate.carpenter_intermediate'"; break;
	case 121: voxAddress = "StaticMesh'/Game/Models/nitrogen_extractor.nitrogen_extractor'"; break;
	case 122: voxAddress = "StaticMesh'/Game/Models/leatherworker.leatherworker'"; break;
	case 123: voxAddress = "StaticMesh'/Game/Models/refinery.refinery'"; break;
	case 124: voxAddress = "StaticMesh'/Game/Models/lever-on.lever-on'"; break;
	case 125: voxAddress = "StaticMesh'/Game/Models/lever-off.lever-off'"; break;
	case 126: voxAddress = "StaticMesh'/Game/Models/pressureplate.pressureplate'"; break;
	case 127: voxAddress = "StaticMesh'/Game/Models/energydoor-open.energydoor-open'"; break;
	case 128: voxAddress = "StaticMesh'/Game/Models/energydoor-closed.energydoor-closed'"; break;
	case 129: voxAddress = "StaticMesh'/Game/Models/spikes-retracted.spikes-retracted'"; break;
	case 130: voxAddress = "StaticMesh'/Game/Models/spikes-extended.spikes-extended'"; break;
	case 131: voxAddress = "StaticMesh'/Game/Models/blades-retracted.blades-retracted'"; break;
	case 132: voxAddress = "StaticMesh'/Game/Models/blades-extended.blades-extended'"; break;
	case 133: voxAddress = "StaticMesh'/Game/Models/stonefall.stonefall'"; break;
	case 134: voxAddress = "StaticMesh'/Game/Models/door_open.door_open'"; break;
	case 135: voxAddress = "StaticMesh'/Game/Models/floodgate_open.floodgate_open'"; break;
	case 136: voxAddress = "StaticMesh'/Game/Models/floodgate_closed.floodgate_closed'"; break;
	case 137: voxAddress = "StaticMesh'/Game/Models/floodgate_ground.floodgate_ground'"; break;
	case 138: voxAddress = "StaticMesh'/Game/Models/float_gauge.float_gauge'"; break;
	case 139: voxAddress = "StaticMesh'/Game/Models/proximity_sensor.proximity_sensor'"; break;
	case 140: voxAddress = "StaticMesh'/Game/Models/oscillator.oscillator'"; break;
	case 141: voxAddress = "StaticMesh'/Game/Models/gate_and.gate_and'"; break;
	case 142: voxAddress = "StaticMesh'/Game/Models/gate_or.gate_or'"; break;
	case 143: voxAddress = "StaticMesh'/Game/Models/gate_not.gate_not'"; break;
	case 144: voxAddress = "StaticMesh'/Game/Models/silicon_refinery.silicon_refinery'"; break;
	case 145: voxAddress = "StaticMesh'/Game/Models/gate_nand.gate_nand'"; break;
	case 146: voxAddress = "StaticMesh'/Game/Models/gate_nor.gate_nor'"; break;
	case 147: voxAddress = "StaticMesh'/Game/Models/gate_xor.gate_xor'"; break;
	case 148: voxAddress = "StaticMesh'/Game/Models/support.support'"; break;
	default: {
		voxAddress = "StaticMesh'/Game/Models/stairs-up.stairs-up'";
		FString comment = TEXT("Unknown voxel model #: ");
		comment.AppendInt(modelId);
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, comment);
	}
	}

	return voxAddress;
}

void ANDisplayManager::InitialBuildings() {
	ecs->ecs.Each<building_t, position_t, name_t>([this](auto &id, building_t &building, position_t &pos, name_t &name) {
		Buildings.Add(id, nullptr);
		FString cname = name.first_name + TEXT("_");
		cname.AppendInt(id);
		Buildings[id] = NewObject<UStaticMeshComponent>(this, FName(*cname));
		FString voxAddress = GetModelPath(building.vox_model);

		UStaticMesh* buildingModel;
		buildingModel = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *voxAddress, nullptr, LOAD_None, nullptr));
		Buildings[id]->SetStaticMesh(buildingModel);
		Buildings[id]->AttachTo(RootComponent);
		Buildings[id]->SetRelativeLocation(FVector((pos.x+0.5f) * WORLD_SCALE, (pos.y+0.5f) * WORLD_SCALE, pos.z * WORLD_SCALE));
		Buildings[id]->RegisterComponent();
	});
}

void ANDisplayManager::InitialComposites() {
	ecs->ecs.Each<renderable_composite_t, position_t, name_t>([this](const int &id, renderable_composite_t &r, position_t &pos, name_t &name) {
		if (!CompositeRender.Contains(id)) {
			const float mx = pos.x + 0.5f;
			const float my = pos.y + 0.5f;
			const float mz = pos.z;

			FRotator rot = FRotator(0, pos.rotation, 0);
			FVector loc = FVector(mx * WORLD_SCALE, my * WORLD_SCALE, mz * WORLD_SCALE);
			FTransform trans = FTransform(rot, loc, FVector(1.0f, 1.0f, 1.0f));
			auto newModel = GetWorld()->SpawnActorDeferred<ANCharacter>(ANCharacter::StaticClass(), trans, this);
			newModel->id = id;
			newModel->CharacterName = name.first_name + TEXT(" ") + name.last_name;
			newModel->FirstName = name.first_name;
			newModel->FinishSpawning(trans);
			newModel->x = pos.x;
			newModel->y = pos.y;
			newModel->z = pos.z;
			newModel->rotation = pos.rotation;
			CompositeRender.Add(id, newModel);
		}
	});
}

void ANDisplayManager::InitialLights() {
	ecs->ecs.Each<lightsource_t, position_t>([this](const int &id, lightsource_t &light, position_t &pos) {
		if (!Lightsources.Contains(id)) {
			FString lightName = TEXT("Light_");
			lightName.AppendInt(id);
			UPointLightComponent * pl = NewObject<UPointLightComponent>(this, FName(*lightName));
			const float mx = (pos.x + 0.5f) * WORLD_SCALE;
			const float my = (pos.y + 0.5f) * WORLD_SCALE;
			const float mz = (pos.z + 0.5f) * WORLD_SCALE;
			pl->SetWorldLocation(FVector(mx, my, mz));
			pl->RegisterComponent();
		}
	});
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GeometryChunk::CreateFloor(int x, int y, int z, int w, int h) {
	const float H = z + 2.0f;
	const float H2 = z + 1.0f;

	// Face up
	vertices.Add(FVector(x + w, y + h, H));
	vertices.Add(FVector(x + w, y, H));
	vertices.Add(FVector(x, y, H));
	vertices.Add(FVector(x, y + h, H));
	vertices.Add(FVector(x + w, y + h, H));
	vertices.Add(FVector(x, y, H));


	Triangles.Add(TriangleCounter);
	Triangles.Add(TriangleCounter + 1);
	Triangles.Add(TriangleCounter + 2);
	Triangles.Add(TriangleCounter + 3);
	Triangles.Add(TriangleCounter + 4);
	Triangles.Add(TriangleCounter + 5);
	TriangleCounter += 6;


	normals.Add(FVector(0, 0, 1));
	normals.Add(FVector(0, 0, 1));
	normals.Add(FVector(0, 0, 1));
	normals.Add(FVector(0, 0, 1));
	normals.Add(FVector(0, 0, 1));
	normals.Add(FVector(0, 0, 1));

	const float tw = (float)w / WORLD_SCALE;
	const float th = (float)h / WORLD_SCALE;

	UV0.Add(FVector2D(tw, th));
	UV0.Add(FVector2D(tw, 0));
	UV0.Add(FVector2D(0, 0));
	UV0.Add(FVector2D(0, th));
	UV0.Add(FVector2D(tw, th));
	UV0.Add(FVector2D(0, 0));

	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));


	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));

	// Face down
	vertices.Add(FVector(x, y, H2));
	vertices.Add(FVector(x + w, y, H2));
	vertices.Add(FVector(x + w, y + h, H2));
	vertices.Add(FVector(x, y, H2));
	vertices.Add(FVector(x + w, y + h, H2));
	vertices.Add(FVector(x, y + h, H2));

	Triangles.Add(TriangleCounter);
	Triangles.Add(TriangleCounter + 1);
	Triangles.Add(TriangleCounter + 2);
	Triangles.Add(TriangleCounter + 3);
	Triangles.Add(TriangleCounter + 4);
	Triangles.Add(TriangleCounter + 5);
	TriangleCounter += 6;


	normals.Add(FVector(0, 0, -1));
	normals.Add(FVector(0, 0, -1));
	normals.Add(FVector(0, 0, -1));
	normals.Add(FVector(0, 0, -1));
	normals.Add(FVector(0, 0, -1));
	normals.Add(FVector(0, 0, -1));

	UV0.Add(FVector2D(0, 0));
	UV0.Add(FVector2D(tw, 0));
	UV0.Add(FVector2D(tw, th));
	UV0.Add(FVector2D(0, 0));
	UV0.Add(FVector2D(tw, th));
	UV0.Add(FVector2D(0, th));

	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));


	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
}

void GeometryChunk::CreateCube(int x, int y, int z, int w, int h, float d) {
	const float tw = (float)w / WORLD_SCALE;
	const float th = (float)h / WORLD_SCALE;

	// Floor
	vertices.Add(FVector(x, y, z));
	vertices.Add(FVector(x + w, y, z));
	vertices.Add(FVector(x + w, y + h, z));
	vertices.Add(FVector(x, y, z));
	vertices.Add(FVector(x + w, y + h, z));
	vertices.Add(FVector(x, y + h, z));


	Triangles.Add(TriangleCounter);
	Triangles.Add(TriangleCounter + 1);
	Triangles.Add(TriangleCounter + 2);
	Triangles.Add(TriangleCounter + 3);
	Triangles.Add(TriangleCounter + 4);
	Triangles.Add(TriangleCounter + 5);
	TriangleCounter += 6;


	normals.Add(FVector(0, 0, -1));
	normals.Add(FVector(0, 0, -1));
	normals.Add(FVector(0, 0, -1));
	normals.Add(FVector(0, 0, -1));
	normals.Add(FVector(0, 0, -1));
	normals.Add(FVector(0, 0, -1));

	UV0.Add(FVector2D(tw, th));
	UV0.Add(FVector2D(tw, 0));
	UV0.Add(FVector2D(0, 0));
	UV0.Add(FVector2D(0, th));
	UV0.Add(FVector2D(tw, th));
	UV0.Add(FVector2D(0, 0));

	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));


	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));

	// Floor
	const float ds = d - 0.05f;
	vertices.Add(FVector(x + w, y + h, z + ds));
	vertices.Add(FVector(x + w, y, z + ds));
	vertices.Add(FVector(x, y, z + ds));
	vertices.Add(FVector(x, y + h, z + ds));
	vertices.Add(FVector(x + w, y + h, z + ds));
	vertices.Add(FVector(x, y, z + ds));

	Triangles.Add(TriangleCounter);
	Triangles.Add(TriangleCounter + 1);
	Triangles.Add(TriangleCounter + 2);
	Triangles.Add(TriangleCounter + 3);
	Triangles.Add(TriangleCounter + 4);
	Triangles.Add(TriangleCounter + 5);
	TriangleCounter += 6;

	normals.Add(FVector(0, 0, 1));
	normals.Add(FVector(0, 0, 1));
	normals.Add(FVector(0, 0, 1));
	normals.Add(FVector(0, 0, 1));
	normals.Add(FVector(0, 0, 1));
	normals.Add(FVector(0, 0, 1));

	UV0.Add(FVector2D(tw, th));
	UV0.Add(FVector2D(tw, 0));
	UV0.Add(FVector2D(0, 0));
	UV0.Add(FVector2D(0, th));
	UV0.Add(FVector2D(tw, th));
	UV0.Add(FVector2D(0, 0));

	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));

	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));

	// Left face
	vertices.Add(FVector(x, y + h, z + ds));
	vertices.Add(FVector(x, y, z + ds));
	vertices.Add(FVector(x, y, z));
	vertices.Add(FVector(x, y + h, z));
	vertices.Add(FVector(x, y + h, z + ds));
	vertices.Add(FVector(x, y, z));

	Triangles.Add(TriangleCounter);
	Triangles.Add(TriangleCounter + 1);
	Triangles.Add(TriangleCounter + 2);
	Triangles.Add(TriangleCounter + 3);
	Triangles.Add(TriangleCounter + 4);
	Triangles.Add(TriangleCounter + 5);
	TriangleCounter += 6;

	normals.Add(FVector(-1, 0, 0));
	normals.Add(FVector(-1, 0, 0));
	normals.Add(FVector(-1, 0, 0));
	normals.Add(FVector(-1, 0, 0));
	normals.Add(FVector(-1, 0, 0));
	normals.Add(FVector(-1, 0, 0));

	UV0.Add(FVector2D(tw, th));
	UV0.Add(FVector2D(tw, 0));
	UV0.Add(FVector2D(0, 0));
	UV0.Add(FVector2D(0, th));
	UV0.Add(FVector2D(tw, th));
	UV0.Add(FVector2D(0, 0));

	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));

	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));

	// Right face
	vertices.Add(FVector(x + w, y, z));
	vertices.Add(FVector(x + w, y, z + ds));
	vertices.Add(FVector(x + w, y + h, z + ds));
	vertices.Add(FVector(x + w, y, z));
	vertices.Add(FVector(x + w, y + h, z + ds));
	vertices.Add(FVector(x + w, y + h, z));

	Triangles.Add(TriangleCounter);
	Triangles.Add(TriangleCounter + 1);
	Triangles.Add(TriangleCounter + 2);
	Triangles.Add(TriangleCounter + 3);
	Triangles.Add(TriangleCounter + 4);
	Triangles.Add(TriangleCounter + 5);
	TriangleCounter += 6;

	normals.Add(FVector(1, 0, 0));
	normals.Add(FVector(1, 0, 0));
	normals.Add(FVector(1, 0, 0));
	normals.Add(FVector(1, 0, 0));
	normals.Add(FVector(1, 0, 0));
	normals.Add(FVector(1, 0, 0));

	UV0.Add(FVector2D(tw, th));
	UV0.Add(FVector2D(tw, 0));
	UV0.Add(FVector2D(0, 0));
	UV0.Add(FVector2D(0, th));
	UV0.Add(FVector2D(tw, th));
	UV0.Add(FVector2D(0, 0));

	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));

	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));

	// North face
	vertices.Add(FVector(x, y, z));
	vertices.Add(FVector(x, y, z + ds));
	vertices.Add(FVector(x + w, y, z + ds));
	vertices.Add(FVector(x, y, z));
	vertices.Add(FVector(x + w, y, z + ds));
	vertices.Add(FVector(x + w, y, z));

	Triangles.Add(TriangleCounter);
	Triangles.Add(TriangleCounter + 1);
	Triangles.Add(TriangleCounter + 2);
	Triangles.Add(TriangleCounter + 3);
	Triangles.Add(TriangleCounter + 4);
	Triangles.Add(TriangleCounter + 5);
	TriangleCounter += 6;

	normals.Add(FVector(-1, 0, 0));
	normals.Add(FVector(-1, 0, 0));
	normals.Add(FVector(-1, 0, 0));
	normals.Add(FVector(-1, 0, 0));
	normals.Add(FVector(-1, 0, 0));
	normals.Add(FVector(-1, 0, 0));

	UV0.Add(FVector2D(0, 0));
	UV0.Add(FVector2D(0, th));
	UV0.Add(FVector2D(tw, th));
	UV0.Add(FVector2D(0, 0));
	UV0.Add(FVector2D(tw, th));
	UV0.Add(FVector2D(tw, 0));

	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));

	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));

	// South face
	vertices.Add(FVector(x + w, y + h, z + ds));
	vertices.Add(FVector(x, y + h, z + ds));
	vertices.Add(FVector(x, y + h, z));
	vertices.Add(FVector(x + w, y + h, z));
	vertices.Add(FVector(x + w, y + h, z + ds));
	vertices.Add(FVector(x, y + h, z));

	Triangles.Add(TriangleCounter);
	Triangles.Add(TriangleCounter + 1);
	Triangles.Add(TriangleCounter + 2);
	Triangles.Add(TriangleCounter + 3);
	Triangles.Add(TriangleCounter + 4);
	Triangles.Add(TriangleCounter + 5);
	TriangleCounter += 6;

	normals.Add(FVector(-1, 0, 0));
	normals.Add(FVector(-1, 0, 0));
	normals.Add(FVector(-1, 0, 0));
	normals.Add(FVector(-1, 0, 0));
	normals.Add(FVector(-1, 0, 0));
	normals.Add(FVector(-1, 0, 0));

	UV0.Add(FVector2D(tw, th));
	UV0.Add(FVector2D(0, th));
	UV0.Add(FVector2D(0, 0));
	UV0.Add(FVector2D(tw, 0));
	UV0.Add(FVector2D(tw, th));
	UV0.Add(FVector2D(0, th));

	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));

	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
}

void GeometryChunk::CreateWater(int x, int y, int z, int w, int h, float d) {
	const float tw = (float)w / WORLD_SCALE;
	const float th = (float)h / WORLD_SCALE;

	// Floor
	vertices.Add(FVector(x, y, z));
	vertices.Add(FVector(x + w, y, z));
	vertices.Add(FVector(x + w, y + h, z));
	vertices.Add(FVector(x, y, z));
	vertices.Add(FVector(x + w, y + h, z));
	vertices.Add(FVector(x, y + h, z));


	Triangles.Add(TriangleCounter);
	Triangles.Add(TriangleCounter + 1);
	Triangles.Add(TriangleCounter + 2);
	Triangles.Add(TriangleCounter + 3);
	Triangles.Add(TriangleCounter + 4);
	Triangles.Add(TriangleCounter + 5);
	TriangleCounter += 6;


	normals.Add(FVector(0, 0, -1));
	normals.Add(FVector(0, 0, -1));
	normals.Add(FVector(0, 0, -1));
	normals.Add(FVector(0, 0, -1));
	normals.Add(FVector(0, 0, -1));
	normals.Add(FVector(0, 0, -1));

	UV0.Add(FVector2D(tw, th));
	UV0.Add(FVector2D(tw, 0));
	UV0.Add(FVector2D(0, 0));
	UV0.Add(FVector2D(0, th));
	UV0.Add(FVector2D(tw, th));
	UV0.Add(FVector2D(0, 0));

	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));


	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));


	// Floor
	const float ds = d - 0.05f;
	vertices.Add(FVector(x + w, y + h, z + ds));
	vertices.Add(FVector(x + w, y, z + ds));
	vertices.Add(FVector(x, y, z + ds));
	vertices.Add(FVector(x, y + h, z + ds));
	vertices.Add(FVector(x + w, y + h, z + ds));
	vertices.Add(FVector(x, y, z + ds));

	Triangles.Add(TriangleCounter);
	Triangles.Add(TriangleCounter + 1);
	Triangles.Add(TriangleCounter + 2);
	Triangles.Add(TriangleCounter + 3);
	Triangles.Add(TriangleCounter + 4);
	Triangles.Add(TriangleCounter + 5);
	TriangleCounter += 6;

	normals.Add(FVector(0, 0, 1));
	normals.Add(FVector(0, 0, 1));
	normals.Add(FVector(0, 0, 1));
	normals.Add(FVector(0, 0, 1));
	normals.Add(FVector(0, 0, 1));
	normals.Add(FVector(0, 0, 1));

	UV0.Add(FVector2D(tw, th));
	UV0.Add(FVector2D(tw, 0));
	UV0.Add(FVector2D(0, 0));
	UV0.Add(FVector2D(0, th));
	UV0.Add(FVector2D(tw, th));
	UV0.Add(FVector2D(0, 0));

	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));

	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));

	/*
	// Left face
	vertices.Add(FVector(x, y + h, z + ds));
	vertices.Add(FVector(x, y, z + ds));
	vertices.Add(FVector(x, y, z));
	vertices.Add(FVector(x, y + h, z));
	vertices.Add(FVector(x, y + h, z + ds));
	vertices.Add(FVector(x, y, z));

	Triangles.Add(TriangleCounter);
	Triangles.Add(TriangleCounter + 1);
	Triangles.Add(TriangleCounter + 2);
	Triangles.Add(TriangleCounter + 3);
	Triangles.Add(TriangleCounter + 4);
	Triangles.Add(TriangleCounter + 5);
	TriangleCounter += 6;

	normals.Add(FVector(-1, 0, 0));
	normals.Add(FVector(-1, 0, 0));
	normals.Add(FVector(-1, 0, 0));
	normals.Add(FVector(-1, 0, 0));
	normals.Add(FVector(-1, 0, 0));
	normals.Add(FVector(-1, 0, 0));

	UV0.Add(FVector2D(tw, th));
	UV0.Add(FVector2D(tw, 0));
	UV0.Add(FVector2D(0, 0));
	UV0.Add(FVector2D(0, th));
	UV0.Add(FVector2D(tw, th));
	UV0.Add(FVector2D(0, 0));

	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));

	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));

	// Right face
	vertices.Add(FVector(x + w, y, z));
	vertices.Add(FVector(x + w, y, z + ds));
	vertices.Add(FVector(x + w, y + h, z + ds));
	vertices.Add(FVector(x + w, y, z));
	vertices.Add(FVector(x + w, y + h, z + ds));
	vertices.Add(FVector(x + w, y + h, z));

	Triangles.Add(TriangleCounter);
	Triangles.Add(TriangleCounter + 1);
	Triangles.Add(TriangleCounter + 2);
	Triangles.Add(TriangleCounter + 3);
	Triangles.Add(TriangleCounter + 4);
	Triangles.Add(TriangleCounter + 5);
	TriangleCounter += 6;

	normals.Add(FVector(1, 0, 0));
	normals.Add(FVector(1, 0, 0));
	normals.Add(FVector(1, 0, 0));
	normals.Add(FVector(1, 0, 0));
	normals.Add(FVector(1, 0, 0));
	normals.Add(FVector(1, 0, 0));

	UV0.Add(FVector2D(tw, th));
	UV0.Add(FVector2D(tw, 0));
	UV0.Add(FVector2D(0, 0));
	UV0.Add(FVector2D(0, th));
	UV0.Add(FVector2D(tw, th));
	UV0.Add(FVector2D(0, 0));

	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));

	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));

	// North face
	vertices.Add(FVector(x, y, z));
	vertices.Add(FVector(x, y, z + ds));
	vertices.Add(FVector(x + w, y, z + ds));
	vertices.Add(FVector(x, y, z));
	vertices.Add(FVector(x + w, y, z + ds));
	vertices.Add(FVector(x + w, y, z));

	Triangles.Add(TriangleCounter);
	Triangles.Add(TriangleCounter + 1);
	Triangles.Add(TriangleCounter + 2);
	Triangles.Add(TriangleCounter + 3);
	Triangles.Add(TriangleCounter + 4);
	Triangles.Add(TriangleCounter + 5);
	TriangleCounter += 6;

	normals.Add(FVector(-1, 0, 0));
	normals.Add(FVector(-1, 0, 0));
	normals.Add(FVector(-1, 0, 0));
	normals.Add(FVector(-1, 0, 0));
	normals.Add(FVector(-1, 0, 0));
	normals.Add(FVector(-1, 0, 0));

	UV0.Add(FVector2D(0, 0));
	UV0.Add(FVector2D(0, th));
	UV0.Add(FVector2D(tw, th));
	UV0.Add(FVector2D(0, 0));
	UV0.Add(FVector2D(tw, th));
	UV0.Add(FVector2D(tw, 0));

	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));

	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));

	// South face
	vertices.Add(FVector(x + w, y + h, z + ds));
	vertices.Add(FVector(x, y + h, z + ds));
	vertices.Add(FVector(x, y + h, z));
	vertices.Add(FVector(x + w, y + h, z));
	vertices.Add(FVector(x + w, y + h, z + ds));
	vertices.Add(FVector(x, y + h, z));

	Triangles.Add(TriangleCounter);
	Triangles.Add(TriangleCounter + 1);
	Triangles.Add(TriangleCounter + 2);
	Triangles.Add(TriangleCounter + 3);
	Triangles.Add(TriangleCounter + 4);
	Triangles.Add(TriangleCounter + 5);
	TriangleCounter += 6;

	normals.Add(FVector(-1, 0, 0));
	normals.Add(FVector(-1, 0, 0));
	normals.Add(FVector(-1, 0, 0));
	normals.Add(FVector(-1, 0, 0));
	normals.Add(FVector(-1, 0, 0));
	normals.Add(FVector(-1, 0, 0));

	UV0.Add(FVector2D(tw, th));
	UV0.Add(FVector2D(0, th));
	UV0.Add(FVector2D(0, 0));
	UV0.Add(FVector2D(tw, 0));
	UV0.Add(FVector2D(tw, th));
	UV0.Add(FVector2D(0, th));

	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));

	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	*/
}

///////////////////////////////////////////////////////////////////////////////////

void ANDisplayManager::TickTock() {
	ecs->GameTick();

	// Destroy emotes
	for (auto &cr : CompositeRender) {
		if (cr.Value->emote != nullptr && cr.Value->emoteTimer > 0) {
			--cr.Value->emoteTimer;
			if (cr.Value->emoteTimer < 1) {
				cr.Value->emote->SetVisibility(false);
				cr.Value->emote->DestroyComponent();
				cr.Value->emote = nullptr;
				cr.Value->emoteTimer = 0;
			}
		}
	}
}

void ANDisplayManager::SetPauseStatus(int p) {
	ecs->SetPauseMode(p);
}

void ANDisplayManager::onCompositeMove(const int id) {
	if (CompositeRender.Contains(id)) {
		auto pos = ecs->ecs.GetComponent<position_t>(id);
		if (pos) {
			const float mx = (pos->x + 0.5f) * WORLD_SCALE;
			const float my = (pos->y + 0.5f) * WORLD_SCALE;
			const float mz = pos->z * WORLD_SCALE;
			CompositeRender[id]->SetActorLocationAndRotation(FVector(mx, my, mz), FRotator(0, pos->rotation + 90.0f, 0));
			CompositeRender[id]->label->SetWorldRotation(FRotator(0, 0, 0));
			if (CompositeRender[id]->emote) CompositeRender[id]->emote->SetWorldRotation(FRotator(0, 0, 0));
		}
	}
}

void ANDisplayManager::onEmote(const int id, const FString text) {
	if (CompositeRender.Contains(id)) {
		CompositeRender[id]->SetEmote(id, text);
	}
}

TArray<FUnitListDisplaySettler> ANDisplayManager::GetSettlerList() {
	TArray<FUnitListDisplaySettler> result;

	ecs->ecs.Each<settler_ai_t, name_t>([&result](const int &entity_id, settler_ai_t &settler, name_t &name) {
		result.Emplace(FUnitListDisplaySettler{ entity_id, name.first_name + TEXT(" ") + name.last_name });
	});

	return result;
}