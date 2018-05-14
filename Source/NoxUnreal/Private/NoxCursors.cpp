// (c) 2016 - Present, Bracket Productions

#include "NoxCursors.h"
#include "../../ThirdParty/libnox/Includes/libnox.h"

// Sets default values
ANoxCursors::ANoxCursors()
{
	mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("CursorMesh"));
	RootComponent = mesh;
	// New in UE 4.17, multi-threaded PhysX cooking.
	mesh->bUseAsyncCooking = true;

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ANoxCursors::BeginPlay()
{
	Super::BeginPlay();

	// Stop cursors from receiving mouse-over events
	mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
}

// Called every frame
void ANoxCursors::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Cursors();
}

void ANoxCursors::Cursors() {
	const float WORLD_SCALE = 200.0f;

	if (!mesh) {
		return;
	}
	mesh->ClearAllMeshSections();
	geometry_by_material.Empty();

	size_t sz;
	nf::cube_t * cube_ptr;
	nf::cursor_list(sz, cube_ptr);

	if (sz > 0) {
		for (size_t i = 0; i < sz; ++i) {
			nf::cube_t cube = cube_ptr[i];

			geometry_chunk * g = nullptr;
			if (!geometry_by_material.Contains(cube.tex)) {
				geometry_by_material.Add(cube.tex, geometry_chunk());
			}
			g = geometry_by_material.Find(cube.tex);

			//geometry.CreateCube(cube.x * WORLD_SCALE, cube.y * WORLD_SCALE, cube.z * WORLD_SCALE, cube.w * WORLD_SCALE, cube.h * WORLD_SCALE, cube.d * WORLD_SCALE);
			g->CreateCube(cube.x * WORLD_SCALE, cube.y * WORLD_SCALE, cube.z * WORLD_SCALE, cube.w * WORLD_SCALE, cube.h * WORLD_SCALE, cube.d * WORLD_SCALE);
		}

		int sectionCount = 0;
		for (auto &gm : geometry_by_material) {
			FString MaterialAddress;

			switch (gm.Key) {
			case 1: MaterialAddress = "Material'/Game/Cursors/base_cursor_mat.base_cursor_mat'"; break; // Normal
			case 2: MaterialAddress = "Material'/Game/Cursors/tree_cursor_mat.tree_cursor_mat'"; break; // Tree chopping
			case 3: MaterialAddress = "Material'/Game/Cursors/guard_cursor_mat.guard_cursor_mat'"; break; // Guarding
			case 4: MaterialAddress = "Material'/Game/Cursors/farm_cursor_mat.farm_cursor_mat'"; break; // Harvest
			case 5: MaterialAddress = "Material'/Game/Cursors/dig_cursor_mat.dig_cursor_mat'"; break; // Dig
			case 6: MaterialAddress = "Material'/Game/Cursors/channel_cursor_mat.channel_cursor_mat'"; break; // Channel
			case 7: MaterialAddress = "Material'/Game/Cursors/ramp_cursor_mat.ramp_cursor_mat'"; break; // Ramp
			case 8: MaterialAddress = "Material'/Game/Cursors/upstairs_cursor_mat.upstairs_cursor_mat'"; break; // Up
			case 9: MaterialAddress = "Material'/Game/Cursors/downstairs_cursor_mat.downstairs_cursor_mat'"; break; // Down
			case 10: MaterialAddress = "Material'/Game/Cursors/updownstairs_cursor_mat.updownstairs_cursor_mat'"; break; // UpDown
			case 11: MaterialAddress = "Material'/Game/Cursors/wall_cursor_mat.wall_cursor_mat'"; break; // Wall
			case 12: MaterialAddress = "Material'/Game/Cursors/floor_cursor_mat.floor_cursor_mat'"; break; // Floor
			case 13: MaterialAddress = "Material'/Game/Cursors/bridge_cursor_mat.bridge_cursor_mat'"; break; // Bridge
			default: MaterialAddress = "Material'/Game/Cursors/base_cursor_mat.base_cursor_mat'"; break;
			}

			UMaterial* material;
			material = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, *MaterialAddress, nullptr, LOAD_None, nullptr));
			mesh->SetMaterial(sectionCount, material);

			mesh->CreateMeshSection_LinearColor(sectionCount, gm.Value.vertices, gm.Value.Triangles, gm.Value.normals, gm.Value.UV0, gm.Value.vertexColors, gm.Value.tangents, true);

			++sectionCount;
		}
	}
}