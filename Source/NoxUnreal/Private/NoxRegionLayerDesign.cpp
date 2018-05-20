// (c) 2016 - Present, Bracket Productions

#include "NoxRegionLayerDesign.h"
#include "../../ThirdParty/libnox/Includes/libnox.h"
#include "NoxLinkTest.h"

// Sets default values
ANoxRegionLayerDesign::ANoxRegionLayerDesign()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));
	RootComponent = mesh;
	// New in UE 4.17, multi-threaded PhysX cooking.
	mesh->bUseAsyncCooking = true;
	mesh->bCastHiddenShadow = true;
}

// Called when the game starts or when spawned
void ANoxRegionLayerDesign::BeginPlay()
{
	Super::BeginPlay();
	ChunkBuilder();
}

// Called every frame
void ANoxRegionLayerDesign::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	int maj, min;
	nf::get_game_mode(maj, min);
	if (maj  == 0) {
		mesh->SetVisibility(false);
		mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		//FoliageVisibility(false);
		return;
	}

	float x, y, z, zoom;
	bool perspective;
	int mode;
	nf::get_camera_position(x, y, z, zoom, perspective, mode);

	// Determine visibility
	const int my_layer = base_z + local_z;
	if (my_layer == z) {
		mesh->SetVisibility(true);
		mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		//FoliageVisibility(true);
	}
	else {
		mesh->SetVisibility(false);
		mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		//FoliageVisibility(false);
	}
}

void ANoxRegionLayerDesign::Rebuild() {
	mesh->ClearAllMeshSections();
	ChunkBuilder();
}

FString GetMaterialDesignTexture(const int &Key) {

	if (link_ptr->MaterialAtlas.Contains(Key) && link_ptr->MaterialAtlas[Key] != "") {
		return link_ptr->MaterialAtlas[Key];
	}
	else {
		return FString(TEXT("MaterialInstanceConstant'/Game/TileMaterials/Instances/M_Alabaster_FloorRough.M_Alabaster_FloorRough'"));
	}
}

void ANoxRegionLayerDesign::ChunkBuilder() {
	geometry_by_material.Empty();
	int z = local_z;
	std::size_t size;
	nf::floor_t * floor_ptr;
	nf::chunk_floors(chunk_idx, z, size, floor_ptr);

	if (size > 0) {
		for (size_t i = 0; i < size; ++i) {
			nf::floor_t floor = floor_ptr[i];

			geometry_chunk * g = nullptr;
			if (!geometry_by_material.Contains(floor.tex)) {
				geometry_by_material.Add(floor.tex, geometry_chunk());
			}
			g = geometry_by_material.Find(floor.tex);

			//geometry.CreateFloor(floor.x * WORLD_SCALE, floor.y * WORLD_SCALE, floor.z * WORLD_SCALE, floor.w * WORLD_SCALE, floor.h * WORLD_SCALE);
			g->CreateFloor(floor.x * WORLD_SCALE, floor.y * WORLD_SCALE, floor.z * WORLD_SCALE, floor.w * WORLD_SCALE, floor.h * WORLD_SCALE);
		}
	}

	nf::cube_t * cube_ptr;
	nf::chunk_cubes(chunk_idx, z, size, cube_ptr);

	if (size > 0) {
		for (size_t i = 0; i < size; ++i) {
			nf::cube_t cube = cube_ptr[i];

			geometry_chunk * g = nullptr;
			if (!geometry_by_material.Contains(cube.tex)) {
				geometry_by_material.Add(cube.tex, geometry_chunk());
			}
			g = geometry_by_material.Find(cube.tex);

			//geometry.CreateCube(cube.x * WORLD_SCALE, cube.y * WORLD_SCALE, cube.z * WORLD_SCALE, cube.w * WORLD_SCALE, cube.h * WORLD_SCALE, cube.d * WORLD_SCALE);
			//g->CreateCube(cube.x * WORLD_SCALE, cube.y * WORLD_SCALE, cube.z * WORLD_SCALE, cube.w * WORLD_SCALE, cube.h * WORLD_SCALE, ((float)cube.d * 0.1f) * WORLD_SCALE);
			g->CreateFloor(cube.x * WORLD_SCALE, cube.y * WORLD_SCALE, cube.z * WORLD_SCALE, cube.w * WORLD_SCALE, cube.h * WORLD_SCALE);
		}
	}

	//mesh->CreateMeshSection_LinearColor(0, geometry.vertices, geometry.Triangles, geometry.normals, geometry.UV0, geometry.vertexColors, geometry.tangents, true);	
	int sectionCount = 0;
	for (auto &gm : geometry_by_material) {
		FString MaterialAddress = GetMaterialDesignTexture(gm.Key);

		UMaterial* material;
		material = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, *MaterialAddress, nullptr, LOAD_None, nullptr));
		mesh->SetMaterial(sectionCount, material);

		mesh->CreateMeshSection_LinearColor(sectionCount, gm.Value.vertices, gm.Value.Triangles, gm.Value.normals, gm.Value.UV0, gm.Value.vertexColors, gm.Value.tangents, true);

		++sectionCount;
	}

	// Enable collision data
	mesh->ContainsPhysicsTriMeshData(true);
}
