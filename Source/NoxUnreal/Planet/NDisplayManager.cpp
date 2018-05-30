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
	}

	// Link to event handlers
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGameInputManager::StaticClass(), FoundActors);
	for (int i = 0; i < FoundActors.Num(); ++i) {
		AGameInputManager * cd = Cast<AGameInputManager>(FoundActors[i]);
		//cd->ZLevelChanged.Broadcast();
		cd->ZLevelChanged.AddDynamic(this, &ANDisplayManager::onZChange);
	}
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
			}
			else {
				Chunks[chunk].layers[layer].mesh->SetVisibility(false);
				Chunks[chunk].layers[layer].mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			}
		}
	}
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