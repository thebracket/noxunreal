// (c) 2016 - Present, Bracket Productions

#include "NoxWorldMap.h"
#include "../Public/NoxGameInstance.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

// Sets default values
ANoxWorldMap::ANoxWorldMap()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));
	RootComponent = mesh;
	mesh->bUseAsyncCooking = true;
	mesh->bCastHiddenShadow = true;
}

inline FVector NormalCalc(const int x, const int y, const float WORLD_SCALE, const float Z_SCALE, UNPlanet * Planet) {
	if (x < 1 || x > nfu::WORLD_WIDTH - 2 || y < 1 || y > nfu::WORLD_HEIGHT - 2) {
		return FVector(0, 0, 1);
	}
	const FVector left = FVector( (x-1)*WORLD_SCALE, y*WORLD_SCALE, Planet->Landblocks[Planet->idx(x-1, y)].height * Z_SCALE );
	const FVector right = FVector((x + 1)*WORLD_SCALE, y*WORLD_SCALE, Planet->Landblocks[Planet->idx(x + 1, y)].height * Z_SCALE);
	const FVector up = FVector(x*WORLD_SCALE, (y-1)*WORLD_SCALE, Planet->Landblocks[Planet->idx(x, y-1)].height * Z_SCALE);
	const FVector down = FVector(x*WORLD_SCALE, (y + 1)*WORLD_SCALE, Planet->Landblocks[Planet->idx(x, y + 1)].height * Z_SCALE);

	const FVector horizontal = left - right;
	const FVector vertical = up - down;
	FVector cross = FVector::CrossProduct(horizontal, vertical);
	cross.Normalize();
	return cross;
}

struct worldgen_map_geometry {
	TArray<FVector> vertices;
	TArray<int32> Triangles;
	TArray<FVector> normals;
	TArray<FVector2D> UV0;
	TArray<FProcMeshTangent> tangents;
	TArray<FLinearColor> vertexColors;
	int TriangleCounter = 0;

	const float WORLD_SCALE = 200.0f;
	const float Z_SCALE = 20.0f;
	const int w = 1 * WORLD_SCALE;
	const int h = 1 * WORLD_SCALE;

	void AddWorldTile(UNPlanet * planet, const int &X, const int &Y, const float z_bonus = 0.0f) {
		const auto &lb = planet->Landblocks[planet->idx(X, Y)];
		const auto &lbr = planet->Landblocks[planet->idx(X + 1, Y)];
		const auto &lbd = planet->Landblocks[planet->idx(X, Y + 1)];
		const auto &lbrd = planet->Landblocks[planet->idx(X + 1, Y + 1)];

		const int x = X * WORLD_SCALE;
		const int y = Y * WORLD_SCALE;
		const int z = lb.height * Z_SCALE;
		const int zr = lbr.height * Z_SCALE;
		const int zd = lbd.height * Z_SCALE;
		const int zrd = lbrd.height * Z_SCALE;

		const float H = z + 2.0f + z_bonus;
		const float H2 = z + 1.0f + z_bonus;
		const float HR = zr + 2.0f + z_bonus;
		const float HD = zd + 2.0f + z_bonus;
		const float HRD = zrd + 2.0f + z_bonus;

		// Face up
		vertices.Add(FVector(x + w, y + h, HRD));
		vertices.Add(FVector(x + w, y, HR));
		vertices.Add(FVector(x, y, H));
		vertices.Add(FVector(x, y + h, HD));
		vertices.Add(FVector(x + w, y + h, HRD));
		vertices.Add(FVector(x, y, H));

		Triangles.Add(TriangleCounter);
		Triangles.Add(TriangleCounter + 1);
		Triangles.Add(TriangleCounter + 2);
		Triangles.Add(TriangleCounter + 3);
		Triangles.Add(TriangleCounter + 4);
		Triangles.Add(TriangleCounter + 5);
		TriangleCounter += 6;

		normals.Add(NormalCalc(X + 1, Y + 1, WORLD_SCALE, Z_SCALE, planet));
		normals.Add(NormalCalc(X + 1, Y, WORLD_SCALE, Z_SCALE, planet));
		normals.Add(NormalCalc(X, Y, WORLD_SCALE, Z_SCALE, planet));
		normals.Add(NormalCalc(X, Y + 1, WORLD_SCALE, Z_SCALE, planet));
		normals.Add(NormalCalc(X + 1, Y + 1, WORLD_SCALE, Z_SCALE, planet));
		normals.Add(NormalCalc(X, Y, WORLD_SCALE, Z_SCALE, planet));

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
	}

	void CreateWater(int x, int y, int z, int w, int h, float d) {
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
	}
};

// Called when the game starts or when spawned
void ANoxWorldMap::BeginPlay()
{
	Super::BeginPlay();

	// Initialize foliage
	UStaticMesh* tree;
	tree = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, TEXT("StaticMesh'/Game/TileMaterials/Foliage/FastTree/tree_1__tree.tree_1__tree'"), nullptr, LOAD_None, nullptr));
	tree1 = NewObject<UHierarchicalInstancedStaticMeshComponent>(this);
	tree1->RegisterComponent();
	tree1->SetStaticMesh(tree);
	//target->SetFlags(RF_Transactional);
	AddInstanceComponent(tree1);

	UStaticMesh* treel;
	treel = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, TEXT("StaticMesh'/Game/TileMaterials/Foliage/FastTree/tree_1__leaves.tree_1__leaves'"), nullptr, LOAD_None, nullptr));
	tree2 = NewObject<UHierarchicalInstancedStaticMeshComponent>(this);
	tree2->RegisterComponent();
	tree2->SetStaticMesh(treel);
	//target->SetFlags(RF_Transactional);
	AddInstanceComponent(tree2);

	UStaticMesh* g;
	g = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, TEXT("StaticMesh'/Game/Meshes/Grass/SM_grass_patch1.SM_grass_patch1'"), nullptr, LOAD_None, nullptr));
	grass = NewObject<UHierarchicalInstancedStaticMeshComponent>(this);
	grass->RegisterComponent();
	grass->SetStaticMesh(g);
	//target->SetFlags(RF_Transactional);
	AddInstanceComponent(grass);

	// Go
	UNoxGameInstance * game = Cast<UNoxGameInstance>(UGameplayStatics::GetGameInstance(this));
	planet = game->GetPlanet();
	raws = game->GetRaws();
	TMap<int32, worldgen_map_geometry> geometry;

	for (int Y = 1; Y < nfu::WORLD_HEIGHT-2; ++Y) {
		for (int X = 1; X < nfu::WORLD_WIDTH-2; ++X) {
			const int idx = planet->idx(X, Y);
			const auto type = planet->Landblocks[idx].type;
			const auto biome_idx = planet->Landblocks[idx].biome_idx;
			const auto biome_type = planet->Biomes[biome_idx].type;

			if (!geometry.Contains(biome_type)) {
				geometry.Add(biome_type, worldgen_map_geometry());
			}
			geometry[biome_type].AddWorldTile(planet, X, Y);

			const auto name = raws->get_biome_def(biome_type)->name;
			if (name.Contains("Evergreen") || name.Contains("Deciduous") || name == "Rainforest") {
				FVector loc = FVector(X * 200.0f, Y * 200.0f, planet->Landblocks[idx].height * 20.0f);
				FTransform trans = FTransform(FRotator(), loc, FVector(30.0, 30.0, 30.0));
				tree1->AddInstance(trans);
				tree2->AddInstance(trans);
			}
			if (name.Contains("Grass")) {
				//FVector loc = FVector(X * 200.0f, Y * 200.0f, planet->Landblocks[idx].height * 20.0f);
				//FTransform trans = FTransform(FRotator(), loc, FVector(6.25, 6.25, 6.25));
				//grass->AddInstance(trans);
			}
		}
	}

	int sectionCount = 0;
	for (auto &g : geometry) {
		FString MaterialAddress;

		const auto biome_idx = g.Key;
		const auto biome_info = raws->get_biome_def(biome_idx);

		if (biome_info) {
			if (biome_info->name == "Grass Plain" || biome_info->name == "Grass Hills" || biome_info->name == "Savannah Hills" || biome_info->name == "Grass Plateau" || biome_info->name == "Grass Mountains" || biome_info->name == "Grass Highlands" || biome_info->name.Contains("Coast") || biome_info->name == "Savannah Mountains") {
				MaterialAddress = "MaterialInstanceConstant'/Game/TileMaterials/Instances/MZ_Grass.MZ_Grass'";
			}
			else if (biome_info->name == "Ocean" || biome_info->name == "Tropical Ocean") {
				MaterialAddress = "MaterialInstanceConstant'/Game/TileMaterials/Instances/MB_Blight.MB_Blight'";
			}
			else if (biome_info->name == "Rocky Plateau" || biome_info->name == "Rocky Plain" || biome_info->name == "Rocky Mountains" || biome_info->name == "Rocky Hills" || biome_info->name == "Rocky Highlands") {
				MaterialAddress = "MaterialInstanceConstant'/Game/TileMaterials/Instances/M_Kaolinite_FloorRough.M_Kaolinite_FloorRough'";
			}
			else if (biome_info->name == "Badlands" || biome_info->name == "Rocky Desert") {
				MaterialAddress = "MaterialInstanceConstant'/Game/TileMaterials/Instances/M_Shale_FloorRough.M_Shale_FloorRough'";
			}
			else if (biome_info->name == "Sand Desert") {
				MaterialAddress = "MaterialInstanceConstant'/Game/TileMaterials/Instances/M_Sandstone_FloorRough.M_Sandstone_FloorRough'";
			}
			else if (biome_info->name == "Icy Ocean" || biome_info->name == "Tundra" || biome_info->name.Contains("Permafrost") || biome_info->name == "Frozen Ocean")
			{
				MaterialAddress = "MaterialInstanceConstant'/Game/TileMaterials/Instances/MZ_Ice.MZ_Ice'";
			} else if (biome_info->name.Contains("Evergreen") || biome_info->name.Contains("Deciduous")) {
				MaterialAddress = "MaterialInstanceConstant'/Game/TileMaterials/Instances/MZ_Grass.MZ_Grass'";
			} else {
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, biome_info->name);
				MaterialAddress = "MaterialInstanceConstant'/Game/TileMaterials/Instances/MZ_Plastic.MZ_Plastic'";
			}
		}
		else {
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("No Biome!"));
			MaterialAddress = "MaterialInstanceConstant'/Game/TileMaterials/Instances/MZ_Plastic.MZ_Plastic'";
		}

		UMaterialInstance* material;
		material = Cast<UMaterialInstance>(StaticLoadObject(UMaterialInstance::StaticClass(), nullptr, *MaterialAddress, nullptr, LOAD_None, nullptr));
		mesh->SetMaterial(sectionCount, material);

		mesh->CreateMeshSection_LinearColor(sectionCount, g.Value.vertices, g.Value.Triangles, g.Value.normals, g.Value.UV0, g.Value.vertexColors, g.Value.tangents, true);
		++sectionCount;
	}

	// Add ocean
	worldgen_map_geometry sealevel;
	sealevel.CreateWater(0, 0, planet->water_height * sealevel.Z_SCALE, nfu::WORLD_WIDTH * sealevel.WORLD_SCALE, nfu::WORLD_HEIGHT * sealevel.WORLD_SCALE, 10.0f);	

	UMaterial* material;
	material = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, TEXT("Material'/Game/Materials/12Water.12Water'"), nullptr, LOAD_None, nullptr));
	mesh->SetMaterial(sectionCount, material);
	mesh->CreateMeshSection_LinearColor(sectionCount, sealevel.vertices, sealevel.Triangles, sealevel.normals, sealevel.UV0, sealevel.vertexColors, sealevel.tangents, true);
	++sectionCount;

	worldgen_map_geometry riviera;
	for (const auto &river : planet->Rivers) {
		if (river.start_x > 2 && river.start_x < nfu::WORLD_WIDTH - 2 && river.start_y > 2 && river.start_y < nfu::WORLD_HEIGHT - 2)
			riviera.AddWorldTile(planet, river.start_x, river.start_y, 10.0f);
		for (const auto &step : river.steps) {
			if (step.x > 2 && step.x < nfu::WORLD_WIDTH-2 && step.y > 2 && step.y < nfu::WORLD_HEIGHT-2)
				riviera.AddWorldTile(planet, step.x, step.y, 10.0f);
		}
	}

	UMaterial* material2;
	material2 = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, TEXT("Material'/Game/Materials/12Water_2.12Water_2'"), nullptr, LOAD_None, nullptr));
	mesh->SetMaterial(sectionCount, material2);
	mesh->CreateMeshSection_LinearColor(sectionCount, riviera.vertices, riviera.Triangles, riviera.normals, riviera.UV0, riviera.vertexColors, riviera.tangents, true);

	mesh->ContainsPhysicsTriMeshData(true);
}

// Called every frame
void ANoxWorldMap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

