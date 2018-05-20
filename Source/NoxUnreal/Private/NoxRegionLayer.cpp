// Fill out your copyright notice in the Description page of Project Settings.

#include "NoxRegionLayer.h"
#include "../../ThirdParty/libnox/Includes/libnox.h"
#include "NoxLinkTest.h"

void geometry_chunk::clear() {
	vertices.Empty();
	Triangles.Empty();
	normals.Empty();
	UV0.Empty();
	tangents.Empty();
	vertexColors.Empty();
	TriangleCounter = 0;
}

// Sets default values
ANoxRegionLayer::ANoxRegionLayer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));
	RootComponent = mesh;
	mesh->bUseAsyncCooking = true;
	mesh->bCastHiddenShadow = true;

	// Create the design mesh
	designMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh2"));
	designMesh->bUseAsyncCooking = true;
	designMesh->bCastHiddenShadow = false;
}

// Called when the game starts or when spawned
void ANoxRegionLayer::BeginPlay()
{
	Super::BeginPlay();
	ChunkBuilder();	
}

void ANoxRegionLayer::onZChange() {
	float x, y, z, zoom;
	bool perspective;
	int mode;
	nf::get_camera_position(x, y, z, zoom, perspective, mode);
	const int my_layer = base_z + local_z;

	int maj, min;
	nf::get_game_mode(maj, min);
	if (maj != 0) {
		if (my_layer == z) {
			mesh->SetVisibility(false);
			mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			FoliageVisibility(true);
			designMesh->SetVisibility(true);
			designMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		}
		else {
			designMesh->SetVisibility(false);
			designMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			FoliageVisibility(false);
			mesh->SetVisibility(false);
			mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		}
		return;
	}
	designMesh->SetVisibility(false);
	designMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);	

	// Determine visibility	
	if (my_layer <= z && my_layer >= z - 20) {
		mesh->SetVisibility(true);
		mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		FoliageVisibility(true);
	}
	else {
		mesh->SetVisibility(false);
		mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		FoliageVisibility(false);
	}
}

// Called every frame
void ANoxRegionLayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);		
}

void ANoxRegionLayer::Rebuild() {
	mesh->ClearAllMeshSections();
	designMesh->ClearAllMeshSections();
	ChunkBuilder();
}

FString GetMaterialTexture(const int &Key) {

	if (link_ptr->MaterialAtlas.Contains(Key) && link_ptr->MaterialAtlas[Key].Len() > 0) {
		return link_ptr->MaterialAtlas[Key];
	}
	else {
		return FString(TEXT("MaterialInstanceConstant'/Game/TileMaterials/Instances/M_Alabaster_FloorRough.M_Alabaster_FloorRough'"));
	}
}

void ANoxRegionLayer::ChunkBuilder() {
	geometry_by_material.Empty();
	design_geometry_by_material.Empty();
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
			g->CreateCube(cube.x * WORLD_SCALE, cube.y * WORLD_SCALE, cube.z * WORLD_SCALE, cube.w * WORLD_SCALE, cube.h * WORLD_SCALE, cube.d * WORLD_SCALE);
		}
	}

	//mesh->CreateMeshSection_LinearColor(0, geometry.vertices, geometry.Triangles, geometry.normals, geometry.UV0, geometry.vertexColors, geometry.tangents, true);	
	int sectionCount = 0;
	for (auto &gm : geometry_by_material) {
		FString MaterialAddress = GetMaterialTexture(gm.Key);

		UMaterialInstance* material;
		material = Cast<UMaterialInstance>(StaticLoadObject(UMaterialInstance::StaticClass(), nullptr, *MaterialAddress, nullptr, LOAD_None, nullptr));
		mesh->SetMaterial(sectionCount, material);

		mesh->CreateMeshSection_LinearColor(sectionCount, gm.Value.vertices, gm.Value.Triangles, gm.Value.normals, gm.Value.UV0, gm.Value.vertexColors, gm.Value.tangents, true);
		++sectionCount;
	}

	// Enable collision data
	mesh->ContainsPhysicsTriMeshData(true);

	// Design mesh - hope this works
	nf::chunk_design_mode(chunk_idx, z, size, floor_ptr);

	if (size > 0) {
		for (size_t i = 0; i < size; ++i) {
			nf::floor_t floor = floor_ptr[i];

			geometry_chunk * g = nullptr;
			if (!design_geometry_by_material.Contains(floor.tex)) {
				design_geometry_by_material.Add(floor.tex, geometry_chunk());
			}
			g = design_geometry_by_material.Find(floor.tex);

			//geometry.CreateFloor(floor.x * WORLD_SCALE, floor.y * WORLD_SCALE, floor.z * WORLD_SCALE, floor.w * WORLD_SCALE, floor.h * WORLD_SCALE);
			g->CreateFloor(floor.x * WORLD_SCALE, floor.y * WORLD_SCALE, floor.z * WORLD_SCALE, floor.w * WORLD_SCALE, floor.h * WORLD_SCALE);
		}
	}
	sectionCount = 0;
	for (auto &gm : design_geometry_by_material) {
		FString MaterialAddress = GetMaterialTexture(gm.Key);

		UMaterial* material;
		material = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, *MaterialAddress, nullptr, LOAD_None, nullptr));
		designMesh->SetMaterial(sectionCount, material);

		designMesh->CreateMeshSection_LinearColor(sectionCount, gm.Value.vertices, gm.Value.Triangles, gm.Value.normals, gm.Value.UV0, gm.Value.vertexColors, gm.Value.tangents, true);
		++sectionCount;
	}
}

void geometry_chunk::CreateFloor(int x, int y, int z, int w, int h) {
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

void geometry_chunk::CreateCube(int x, int y, int z, int w, int h, float d) {
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

void geometry_chunk::CreateWater(int x, int y, int z, int w, int h, float d) {
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

void ANoxRegionLayer::FoliageInit(FString &voxAddress, UHierarchicalInstancedStaticMeshComponent *& target) {
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


void ANoxRegionLayer::FoliageVisibility(const bool &vis) {
	grass1->SetVisibility(vis);
	flower1->SetVisibility(vis);
	bush1->SetVisibility(vis);
	bush2->SetVisibility(vis);
	grass2->SetVisibility(vis);
	tree1->SetVisibility(vis);
}

void ANoxRegionLayer::InitializeFoliageContainers() {
	FString grassPatch = FString("StaticMesh'/Game/Foliage/Meshes/Grass/SM_grass_patch1.SM_grass_patch1'");
	FString flowerPatch = FString("StaticMesh'/Game/Foliage/Meshes/Flowers/SM_grass_patch_flower1.SM_grass_patch_flower1'");
	FString bushPatch = FString("StaticMesh'/Game/Foliage/Meshes/Bush/SM_bush1.SM_bush1'");
	FString fatBush = FString("StaticMesh'/Game/Foliage/Meshes/Bush/SM_bush4.SM_bush4'");
	FString grassPatch2 = FString("StaticMesh'/Game/Foliage/Meshes/Grass/SM_grass_patch6.SM_grass_patch6'");
	FString treeModel = FString("StaticMesh'/Game/Foliage/Meshes/Trees/SM_tree1.SM_tree1'");

	FoliageInit(grassPatch, grass1);
	FoliageInit(flowerPatch, flower1);
	FoliageInit(bushPatch, bush1);
	FoliageInit(fatBush, bush2);
	FoliageInit(grassPatch2, grass2);
	FoliageInit(treeModel, tree1);

	UMaterial * mat1 = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, TEXT("Material'/Game/Materials/06Bark.06Bark'"), nullptr, LOAD_None, nullptr));
	UMaterial * mat2 = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, TEXT("Material'/Game/Materials/09Leaf.09Leaf'"), nullptr, LOAD_None, nullptr));

	tree1->SetMaterial(0, mat1);
	tree1->SetMaterial(1, mat2);
}

void ANoxRegionLayer::FoliageClear() {
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
	case -1: return FVector(1.0f, 1.0f, 1.0f); // Tree
	case 0: return FVector(2.0f, 2.0f, 0.6f); // Artichoke
	case 1: return FVector(6.25f, 5.6f, 1.0f); // Asparagus
	case 2: return FVector(2.0f, 2.0f, 0.4f); // Bambara
	case 3: return FVector(6.25f, 5.6f, 0.1f); // Beetroot
	case 4: return FVector(2.0f, 2.0f, 0.3f); // Broad Bean
	case 6: return FVector(1.5f, 1.5f, 0.3f); // Cabbage
	case 7: return FVector(1.7f, 1.7f, 0.4f); // Caper
	case 9: return FVector(2.0f, 2.0f, 0.8f); // Cassava
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

void ANoxRegionLayer::FoliageSieve(nf::veg_t &model) {
	using namespace impl;
	const float mx = model.x + 0.5f;
	const float my = model.y + 0.5f;
	const float mz = model.z;

	FVector loc = FVector(mx * 200, my * 200, mz * 200);

	UHierarchicalInstancedStaticMeshComponent * target;

	switch (model.plant) {
	case -1: target = tree1; break; // Special case for trees
	case 0: target = bush2; break; // Artichoke
	case 2: target = bush2; break; // Bambara
	case 3: target = flower1; break; // Beetroot
	case 4: target = bush2; break; // Broad bean
	case 6: target = bush1; break; // Cabbage
	case 7: target = bush1; break; // Caper
	case 8: target = grass2; break; // Carrot
	case 9: target = bush2; break; // Cassava
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

void ANoxRegionLayer::StaticFoliage(size_t &size, nf::veg_t *& veg_ptr)
{
	InitializeFoliageContainers();

	FoliageClear();

	if (size > 0) {
		for (size_t i = 0; i < size; ++i) {
			nf::veg_t model = veg_ptr[i];
			if (model.z == local_z+base_z) {
				FoliageSieve(model);
			}
		}
	}
}
