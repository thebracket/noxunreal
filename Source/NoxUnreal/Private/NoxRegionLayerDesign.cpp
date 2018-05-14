// (c) 2016 - Present, Bracket Productions

#include "NoxRegionLayerDesign.h"
#include "../../ThirdParty/libnox/Includes/libnox.h"

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
		FString MaterialAddress;

		switch (gm.Key) {
		case 0: MaterialAddress = "Material'/Game/Materials/00Grass.00Grass'"; break;
		case 3: MaterialAddress = "Material'/Game/Materials/03Plastic.03Plastic'"; break;
		case 6: MaterialAddress = "Material'/Game/Materials/06Bark.06Bark'"; break;
		case 9: MaterialAddress = "Material'/Game/Materials/09Leaf.09Leaf'"; break;
		case 12: MaterialAddress = "Material'/Game/Materials/12Water.12Water'"; break;
		case 15: MaterialAddress = "Material'/Game/Materials/15Window.15Window'"; break;
		case 18: MaterialAddress = "Material'/Game/Materials/18Germinating.18Germinating'"; break;
		case 21: MaterialAddress = "Material'/Game/Materials/21Sprouting.21Sprouting'"; break;
		case 24: MaterialAddress = "Material'/Game/Materials/24Flowering.24Flowering'"; break;
		case 27: MaterialAddress = "Material'/Game/Materials/27Sandstone.27Sandstone'"; break;
		case 30: MaterialAddress = "Material'/Game/Materials/30CopperRock.30CopperRock'"; break;
		case 33: MaterialAddress = "Material'/Game/Materials/33RedRock.33RedRock'"; break;
		case 36: MaterialAddress = "Material'/Game/Materials/36Limestone.36Limestone'"; break;
		case 39: MaterialAddress = "Material'/Game/Materials/39Granite.39Granite'"; break;

		case 42: MaterialAddress = "Material'/Game/Materials/42GreyPocked.42GreyPocked'"; break;
		case 45: MaterialAddress = "Material'/Game/Materials/45YellowPocked.45YellowPocked'"; break;
		case 48: MaterialAddress = "Material'/Game/Materials/48BluePocked.48BluePocked'"; break;
		case 51: MaterialAddress = "Material'/Game/Materials/51GreenPocked.51GreenPocked'"; break;
		case 54: MaterialAddress = "Material'/Game/Materials/54Slate.54Slate'"; break;
		case 57: MaterialAddress = "Material'/Game/Materials/57BlackRock.57BlackRock'"; break;
		case 60: MaterialAddress = "Material'/Game/Materials/60BlocksRough.60BlocksRough'"; break;
		case 63: MaterialAddress = "Material'/Game/Materials/63Cobbles.63Cobbles'"; break;
		case 66: MaterialAddress = "Material'/Game/Materials/66SandySoil.66SandySoil'"; break;
		case 69: MaterialAddress = "Material'/Game/Materials/69DryDirt.69DryDirt'"; break;
		case 72: MaterialAddress = "Material'/Game/Materials/72DryBrownDirt.72DryBrownDirt'"; break;
		case 75: MaterialAddress = "Material'/Game/Materials/75Rubber.75Rubber'"; break;
		case 78: MaterialAddress = "Material'/Game/Materials/78Titanium.78Titanium'"; break;
		case 81: MaterialAddress = "Material'/Game/Materials/81Wood.81Wood'"; break;
		case 84: MaterialAddress = "Material'/Game/Materials/84SlicedRock.84SlicedRock'"; break;
		case 87: MaterialAddress = "Material'/Game/Materials/87StreakedMarble.87StreakedMarble'"; break;
		case 90: MaterialAddress = "Material'/Game/Materials/90CaveFloor.90CaveFloor'"; break;
		case 93: MaterialAddress = "Material'/Game/Materials/93CaveFloorRed.93CaveFloorRed'"; break;
		case 96: MaterialAddress = "Material'/Game/Materials/96StreakedMarbleRed.96StreakedMarbleRed'"; break;
		case 99: MaterialAddress = "Material'/Game/Materials/99StreakedMarbleBlue.99StreakedMarbleBlue'"; break;
		case 102: MaterialAddress = "Material'/Game/Materials/102SlipperyStone.102SlipperyStone'"; break;
		case 105: MaterialAddress = "Material'/Game/Materials/105SlidedRockBlack.105SlidedRockBlack'"; break;
		case 108: MaterialAddress = "Material'/Game/Materials/108SlicedRockYellow.108SlicedRockYellow'"; break;
		case 111: MaterialAddress = "Material'/Game/Materials/111SlicedRockBlue.111SlicedRockBlue'"; break;
		case 114: MaterialAddress = "Material'/Game/Materials/114SlipperyStoneYellow.114SlipperyStoneYellow'"; break;
		case 117: MaterialAddress = "Material'/Game/Materials/117Cement.117Cement'"; break;
		case 120: MaterialAddress = "Material'/Game/Materials/120OakFloor.120OakFloor'"; break;
		case 123: MaterialAddress = "Material'/Game/Materials/123PlasteelFloor.123PlasteelFloor'"; break;
		case 126: MaterialAddress = "Material'/Game/Materials/126CeramicTile.126CeramicTile'"; break;
		case 129: MaterialAddress = "Material'/Game/Materials/129PlasteelTile.129PlasteelTile'"; break;
		case 132: MaterialAddress = "Material'/Game/Materials/132MetalFloor.132MetalFloor'"; break;
		case 135: MaterialAddress = "Material'/Game/Materials/135Blight.135Blight'"; break;
		default: MaterialAddress = "Material'/Game/Materials/00Grass.00Grass'";
		}

		UMaterial* material;
		material = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, *MaterialAddress, nullptr, LOAD_None, nullptr));
		mesh->SetMaterial(sectionCount, material);

		mesh->CreateMeshSection_LinearColor(sectionCount, gm.Value.vertices, gm.Value.Triangles, gm.Value.normals, gm.Value.UV0, gm.Value.vertexColors, gm.Value.tangents, true);

		++sectionCount;
	}

	// Enable collision data
	mesh->ContainsPhysicsTriMeshData(true);
}
