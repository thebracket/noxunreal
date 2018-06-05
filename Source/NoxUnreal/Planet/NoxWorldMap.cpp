// (c) 2016 - Present, Bracket Productions

#include "NoxWorldMap.h"
#include "../Public/NoxGameInstance.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

constexpr float zscaler = 0.0f;

// Sets default values
ANoxWorldMap::ANoxWorldMap()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	scene = CreateDefaultSubobject<USceneComponent>("Scene");
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
	const float Z_SCALE = zscaler;
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

	mesh = NewObject<UProceduralMeshComponent>(this, TEXT("GeneratedMesh"));

	RootComponent = scene;
	mesh->bUseAsyncCooking = true;
	mesh->bCastHiddenShadow = true;
	mesh->AttachTo(RootComponent);
	mesh->RegisterComponent();

	// Go
	UNoxGameInstance * game = Cast<UNoxGameInstance>(UGameplayStatics::GetGameInstance(this));
	planet = game->GetPlanet();
	raws = game->GetRaws();
	TMap<int32, worldgen_map_geometry> geometry;

	for (int Y = 1; Y < nfu::WORLD_HEIGHT - 2; ++Y) {
		for (int X = 1; X < nfu::WORLD_WIDTH - 2; ++X) {
			const int idx = planet->idx(X, Y);
			const auto type = planet->Landblocks[idx].type;
			const auto biome_idx = planet->Landblocks[idx].biome_idx;
			const auto biome_type = planet->Landblocks[idx].height < planet->water_height ? -1 : planet->Biomes[biome_idx].type;

			if (!geometry.Contains(biome_type)) {
				geometry.Add(biome_type, worldgen_map_geometry());
			}
			geometry[biome_type].AddWorldTile(planet, X, Y);

		}
	}
	RefreshModels();

	int sectionCount = 0;
	for (auto &g : geometry) {
		FString MaterialAddress;

		const auto biome_idx = g.Key;
		if (biome_idx == -1) {
			MaterialAddress = "MaterialInstanceConstant'/Game/WorldMap/Materials/WM_GrassPlain.WM_GrassPlain'";
		}
		const auto biome_info = raws->get_biome_def(biome_idx);

		if (biome_info && biome_idx > -1) {
			if (biome_info->name == "Grass Plain" || biome_info->name == "Temperate Coast" || biome_info->name == "Ocean") {
				MaterialAddress = "MaterialInstanceConstant'/Game/WorldMap/Materials/WM_GrassPlain.WM_GrassPlain'";
			}
			else if (biome_info->name == "Savannah Plain" || biome_info->name == "Tropical Coast" || biome_info->name == "Hot Ocean") {
				MaterialAddress = "MaterialInstanceConstant'/Game/WorldMap/Materials/WM_Savannah_Plain.WM_Savannah_Plain'";
			}
			else if (biome_info->name == "Rocky Plain" || biome_info->name == "Cold Coast" || biome_info->name == "Icy Ocean") {
				MaterialAddress = "MaterialInstanceConstant'/Game/WorldMap/Materials/WM_Rocky_Plain.WM_Rocky_Plain'";
			}
			else if (biome_info->name == "Permafrost Plain" || biome_info->name == "Arctic Coast" || biome_info->name == "Frozen Ocean") {
				MaterialAddress = "MaterialInstanceConstant'/Game/WorldMap/Materials/WM_Permafrost_Plain.WM_Permafrost_Plain'";
			}
			else if (biome_info->name == "Permafrost Plateau") {
				MaterialAddress = "MaterialInstanceConstant'/Game/WorldMap/Materials/WM_Permafrost_Plateau.WM_Permafrost_Plateau'";
			}
			else if (biome_info->name == "Rocky Plateau") {
				MaterialAddress = "MaterialInstanceConstant'/Game/WorldMap/Materials/WM_Rocky_Plateau.WM_Rocky_Plateau'";
			}
			else if (biome_info->name == "Grass Plateau") {
				MaterialAddress = "MaterialInstanceConstant'/Game/WorldMap/Materials/WM_Grass_Plateau.WM_Grass_Plateau'";
			}
			else if (biome_info->name == "Savannah Plateau") {
				MaterialAddress = "MaterialInstanceConstant'/Game/WorldMap/Materials/WM_Savannah_Plateau.WM_Savannah_Plateau'";
			}
			else if (biome_info->name == "Permafrost Hills") {
				MaterialAddress = "MaterialInstanceConstant'/Game/WorldMap/Materials/WM_Permafrost_Hills.WM_Permafrost_Hills'";
			}
			else if (biome_info->name == "Rocky Hills") {
				MaterialAddress = "MaterialInstanceConstant'/Game/WorldMap/Materials/WM_Rocky_Hills.WM_Rocky_Hills'";
			}
			else if (biome_info->name == "Grass Hills") {
				MaterialAddress = "MaterialInstanceConstant'/Game/WorldMap/Materials/WM_Grass_Hills.WM_Grass_Hills'";
			}
			else if (biome_info->name == "Savannah Hills") {
				MaterialAddress = "MaterialInstanceConstant'/Game/WorldMap/Materials/WM_Savannah_Hills.WM_Savannah_Hills'";
			}
			else if (biome_info->name == "Permafrost Highlands") {
				MaterialAddress = "MaterialInstanceConstant'/Game/WorldMap/Materials/WM_Permafrost_Highlands.WM_Permafrost_Highlands'";
			}
			else if (biome_info->name == "Rocky Highlands") {
				MaterialAddress = "MaterialInstanceConstant'/Game/WorldMap/Materials/WM_Rocky_Highlands.WM_Rocky_Highlands'";
			}
			else if (biome_info->name == "Grass Highlands") {
				MaterialAddress = "MaterialInstanceConstant'/Game/WorldMap/Materials/WM_Grass_Highlands.WM_Grass_Highlands'";
			}
			else if (biome_info->name == "Savannah Highlands") {
				MaterialAddress = "MaterialInstanceConstant'/Game/WorldMap/Materials/WM_Savannah_Highlands.WM_Savannah_Highlands'";
			}
			else if (biome_info->name == "Badland Highlands") {
				MaterialAddress = "MaterialInstanceConstant'/Game/WorldMap/Materials/WM_Badland_high.WM_Badland_high'";
			}
			else if (biome_info->name == "Permafrost Mountains") {
				MaterialAddress = "MaterialInstanceConstant'/Game/WorldMap/Materials/WM_Permafrost_Mountains.WM_Permafrost_Mountains'";
			}
			else if (biome_info->name == "Rocky Mountains") {
				MaterialAddress = "MaterialInstanceConstant'/Game/WorldMap/Materials/WM_Rocky_Mountains.WM_Rocky_Mountains'";
			}
			else if (biome_info->name == "Grass Mountains") {
				MaterialAddress = "MaterialInstanceConstant'/Game/WorldMap/Materials/WM_Grass_Mountains.WM_Grass_Mountains'";
			}
			else if (biome_info->name == "Savannah Mountains") {
				MaterialAddress = "MaterialInstanceConstant'/Game/WorldMap/Materials/WM_Savannah_Mountains.WM_Savannah_Mountains'";
			}
			else if (biome_info->name == "Tundra") {
				MaterialAddress = "MaterialInstanceConstant'/Game/WorldMap/Materials/WM_Tundra.WM_Tundra'";
			}
			else if (biome_info->name == "Cold Desert") {
				MaterialAddress = "MaterialInstanceConstant'/Game/WorldMap/Materials/WM_ColdDesert.WM_ColdDesert'";
			}
			else if (biome_info->name == "Sand Desert") {
				MaterialAddress = "MaterialInstanceConstant'/Game/WorldMap/Materials/WM_SandDesert.WM_SandDesert'";
			}
			else if (biome_info->name == "Rocky Desert") {
				MaterialAddress = "MaterialInstanceConstant'/Game/WorldMap/Materials/WM_Rocky_Desert.WM_Rocky_Desert'";
			}
			else if (biome_info->name == "Badlands") {
				MaterialAddress = "MaterialInstanceConstant'/Game/WorldMap/Materials/WM_Rocky_Badlands.WM_Rocky_Badlands'";
			}
			else if (biome_info->name == "Deciduous Broadleaf" || biome_info->name == "Deciduous Needleleaf" || biome_info->name == "Deciduous Needleleaf" || biome_info->name == "Evergreen Broadleaf" || biome_info->name == "Evergreen Needleleaf" || biome_info->name == "Rainforest") {
				MaterialAddress = "MaterialInstanceConstant'/Game/WorldMap/Materials/WM_Trees.WM_Trees'";
			}
			else {
				//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, biome_info->name);
				MaterialAddress = "MaterialInstanceConstant'/Game/Materials/12Water_I.12Water_I'";
			}
		}
		else {
			//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("No Biome!"));
			MaterialAddress = "MaterialInstanceConstant'/Game/Materials/12Water_I.12Water_I'";
		}

		UMaterialInstance* material;
		material = Cast<UMaterialInstance>(StaticLoadObject(UMaterialInstance::StaticClass(), nullptr, *MaterialAddress, nullptr, LOAD_None, nullptr));
		mesh->SetMaterial(sectionCount, material);

		mesh->CreateMeshSection_LinearColor(sectionCount, g.Value.vertices, g.Value.Triangles, g.Value.normals, g.Value.UV0, g.Value.vertexColors, g.Value.tangents, true);
		++sectionCount;
	}

	mesh->ContainsPhysicsTriMeshData(true);
}

// Called every frame
void ANoxWorldMap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ANoxWorldMap::AddFeatureToMap(const int &mapidx, UStaticMeshComponent * mesh) {
	if (!features.Contains(mapidx)) {
		features.Add(mapidx, TArray<UStaticMeshComponent *>());
	}
	features[mapidx].Add(mesh);
}

void ANoxWorldMap::RefreshModels() {
	for (auto &f : features) {
		for (auto c : f.Value) {
			c->DestroyComponent();
		}
	}

	features.Empty();

	for (int Y = 1; Y < nfu::WORLD_HEIGHT - 2; ++Y) {
		for (int X = 1; X < nfu::WORLD_WIDTH - 2; ++X) {
			const int idx = planet->idx(X, Y);
			const auto type = planet->Landblocks[idx].type;
			const auto biome_idx = planet->Landblocks[idx].biome_idx;
			const auto biome_type = planet->Landblocks[idx].height < planet->water_height ? -1 : planet->Biomes[biome_idx].type;

			const float WATER_SCALE = 1.00;
			const float MX = X + 0.5f;
			const float MY = Y + 0.5f;
			const float MZ = 10.0f;

			const auto RiverType = planet->Landblocks[idx].RiverMask;
			if (RiverType > 0 && planet->Landblocks[idx].height > planet->water_height) {
				if (testbit(RiverMaskBits::START, planet->Landblocks[idx].RiverMask) || planet->Landblocks[idx].RiverMask == 15) {
					FVector loc = FVector(MX * 200.0f, MY * 200.0f, MZ);
					FTransform trans = FTransform(FRotator(), loc, FVector(WATER_SCALE, WATER_SCALE, WATER_SCALE));
					UStaticMeshComponent * pool = NewObject<UStaticMeshComponent>(this);
					pool->SetStaticMesh(Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, TEXT("StaticMesh'/Game/WorldMap/Models/wm_pool.wm_pool'"), nullptr, LOAD_None, nullptr)));
					pool->SetRelativeLocation(loc);
					pool->SetRelativeScale3D(FVector(WATER_SCALE, WATER_SCALE, WATER_SCALE));
					pool->SetMobility(EComponentMobility::Movable);
					pool->AttachTo(RootComponent);
					pool->RegisterComponent();
					AddFeatureToMap(idx, pool);
				}
				if (testbit(RiverMaskBits::NORTH, planet->Landblocks[idx].RiverMask)) {
					FVector loc = FVector(MX * 200.0f, MY * 200.0f, MZ);
					FTransform trans = FTransform(FRotator(), loc, FVector(WATER_SCALE, WATER_SCALE, WATER_SCALE));

					UStaticMeshComponent * pool = NewObject<UStaticMeshComponent>(this);
					pool->SetStaticMesh(Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, TEXT("StaticMesh'/Game/WorldMap/Models/wm_river.wm_river'"), nullptr, LOAD_None, nullptr)));
					pool->SetRelativeLocation(loc);
					pool->SetRelativeScale3D(FVector(WATER_SCALE, WATER_SCALE, WATER_SCALE));
					pool->SetWorldRotation(FRotator(0.0f, 90.0f, 0.0f));
					pool->SetMobility(EComponentMobility::Movable);
					pool->AttachTo(RootComponent);
					pool->RegisterComponent();
					AddFeatureToMap(idx, pool);
				}
				if (testbit(RiverMaskBits::SOUTH, planet->Landblocks[idx].RiverMask)) {
					FVector loc = FVector(MX * 200.0f, MY * 200.0f, MZ);
					FTransform trans = FTransform(FRotator(), loc, FVector(WATER_SCALE, WATER_SCALE, WATER_SCALE));

					UStaticMeshComponent * pool = NewObject<UStaticMeshComponent>(this);
					pool->SetStaticMesh(Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, TEXT("StaticMesh'/Game/WorldMap/Models/wm_river.wm_river'"), nullptr, LOAD_None, nullptr)));
					pool->SetRelativeLocation(loc);
					pool->SetRelativeScale3D(FVector(WATER_SCALE, WATER_SCALE, WATER_SCALE));
					pool->SetRelativeRotation(FRotator(0.0f, 270.0f, 0.0f));
					pool->SetMobility(EComponentMobility::Movable);
					pool->AttachTo(RootComponent);
					pool->RegisterComponent();
					AddFeatureToMap(idx, pool);
				}
				if (testbit(RiverMaskBits::EAST, planet->Landblocks[idx].RiverMask)) {
					FVector loc = FVector(MX * 200.0f, MY * 200.0f, MZ);
					FTransform trans = FTransform(FRotator(), loc, FVector(WATER_SCALE, WATER_SCALE, WATER_SCALE));

					UStaticMeshComponent * pool = NewObject<UStaticMeshComponent>(this);
					pool->SetStaticMesh(Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, TEXT("StaticMesh'/Game/WorldMap/Models/wm_river.wm_river'"), nullptr, LOAD_None, nullptr)));
					pool->SetRelativeLocation(loc);
					pool->SetRelativeScale3D(FVector(WATER_SCALE, WATER_SCALE, WATER_SCALE));
					pool->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
					pool->SetMobility(EComponentMobility::Movable);
					pool->AttachTo(RootComponent);
					pool->RegisterComponent();
					AddFeatureToMap(idx, pool);
				}
				if (testbit(RiverMaskBits::WEST, planet->Landblocks[idx].RiverMask)) {
					FVector loc = FVector(MX * 200.0f, MY * 200.0f, MZ);
					FTransform trans = FTransform(FRotator(), loc, FVector(WATER_SCALE, WATER_SCALE, WATER_SCALE));

					UStaticMeshComponent * pool = NewObject<UStaticMeshComponent>(this);
					pool->SetStaticMesh(Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, TEXT("StaticMesh'/Game/WorldMap/Models/wm_river.wm_river'"), nullptr, LOAD_None, nullptr)));
					pool->SetRelativeLocation(loc);
					pool->SetRelativeScale3D(FVector(WATER_SCALE, WATER_SCALE, WATER_SCALE));
					//pool->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
					pool->SetMobility(EComponentMobility::Movable);
					pool->AttachTo(RootComponent);
					pool->RegisterComponent();
					AddFeatureToMap(idx, pool);
				}
			}

			if (planet->Landblocks[idx].height > planet->water_height) {
				const auto name = raws->get_biome_def(biome_type)->name;
				if ((name.Contains("Evergreen") || name.Contains("Deciduous") || name == "Rainforest") && planet->Landblocks[idx].Features == 0) {
					FVector loc = FVector(MX * 200.0f, MY * 200.0f, MZ);
					FTransform trans = FTransform(FRotator(), loc, FVector(30.0, 30.0, 30.0));

					UStaticMeshComponent * tree = NewObject<UStaticMeshComponent>(this);
					tree->SetStaticMesh(Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, TEXT("StaticMesh'/Game/TileMaterials/Foliage/FastTree/tree_1__tree.tree_1__tree'"), nullptr, LOAD_None, nullptr)));
					tree->SetRelativeLocation(loc);
					tree->SetRelativeScale3D(FVector(30.0, 30.0, 30.0));
					tree->AttachTo(RootComponent);
					tree->RegisterComponent();
					AddFeatureToMap(idx, tree);

					UStaticMeshComponent * tree2 = NewObject<UStaticMeshComponent>(this);
					tree2->SetStaticMesh(Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, TEXT("StaticMesh'/Game/TileMaterials/Foliage/FastTree/tree_1__leaves.tree_1__leaves'"), nullptr, LOAD_None, nullptr)));
					tree2->SetRelativeLocation(loc);
					tree2->SetRelativeScale3D(FVector(30.0, 30.0, 30.0));
					tree2->AttachTo(RootComponent);
					tree2->RegisterComponent();
					AddFeatureToMap(idx, tree2);
				}

				if (testbit(FeatureMaskBits::HUTS, planet->Landblocks[idx].Features)) {
					FVector loc = FVector(MX * 200.0f, MY * 200.0f, MZ);
					UStaticMeshComponent * tree2 = NewObject<UStaticMeshComponent>(this);
					tree2->SetStaticMesh(Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, TEXT("StaticMesh'/Game/WorldMap/Models/wm_hut.wm_hut'"), nullptr, LOAD_None, nullptr)));
					tree2->SetRelativeLocation(loc);
					tree2->SetRelativeScale3D(FVector(WATER_SCALE, WATER_SCALE, WATER_SCALE));
					tree2->AttachTo(RootComponent);
					tree2->RegisterComponent();
					AddFeatureToMap(idx, tree2);
				}

				if (testbit(FeatureMaskBits::FARM, planet->Landblocks[idx].Features)) {
					FVector loc = FVector(MX * 200.0f, MY * 200.0f, MZ);
					UStaticMeshComponent * tree2 = NewObject<UStaticMeshComponent>(this);
					tree2->SetStaticMesh(Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, TEXT("StaticMesh'/Game/WorldMap/Models/wm_farm.wm_farm'"), nullptr, LOAD_None, nullptr)));
					tree2->SetRelativeLocation(loc);
					tree2->SetRelativeScale3D(FVector(WATER_SCALE, WATER_SCALE, WATER_SCALE));
					tree2->AttachTo(RootComponent);
					tree2->RegisterComponent();
					AddFeatureToMap(idx, tree2);
				}
				if (testbit(FeatureMaskBits::MINE, planet->Landblocks[idx].Features)) {
					FVector loc = FVector(MX * 200.0f, MY * 200.0f, MZ);
					UStaticMeshComponent * tree2 = NewObject<UStaticMeshComponent>(this);
					tree2->SetStaticMesh(Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, TEXT("StaticMesh'/Game/WorldMap/Models/wm_mine.wm_mine'"), nullptr, LOAD_None, nullptr)));
					tree2->SetRelativeLocation(loc);
					tree2->SetRelativeScale3D(FVector(WATER_SCALE, WATER_SCALE, WATER_SCALE));
					tree2->AttachTo(RootComponent);
					tree2->RegisterComponent();
					AddFeatureToMap(idx, tree2);
				}
				if (testbit(FeatureMaskBits::LUMBER, planet->Landblocks[idx].Features)) {
					FVector loc = FVector(MX * 200.0f, MY * 200.0f, MZ);
					UStaticMeshComponent * tree2 = NewObject<UStaticMeshComponent>(this);
					tree2->SetStaticMesh(Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, TEXT("StaticMesh'/Game/WorldMap/Models/wm_lumber.wm_lumber'"), nullptr, LOAD_None, nullptr)));
					tree2->SetRelativeLocation(loc);
					tree2->SetRelativeScale3D(FVector(WATER_SCALE, WATER_SCALE, WATER_SCALE));
					tree2->AttachTo(RootComponent);
					tree2->RegisterComponent();
					AddFeatureToMap(idx, tree2);
				}
				if (testbit(FeatureMaskBits::PALLISADE, planet->Landblocks[idx].Features)) {
					FVector loc = FVector(MX * 200.0f, MY * 200.0f, MZ);
					UStaticMeshComponent * tree2 = NewObject<UStaticMeshComponent>(this);
					tree2->SetStaticMesh(Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, TEXT("StaticMesh'/Game/WorldMap/Models/wm_pallisade.wm_pallisade'"), nullptr, LOAD_None, nullptr)));
					tree2->SetRelativeLocation(loc);
					tree2->SetRelativeScale3D(FVector(WATER_SCALE, WATER_SCALE, WATER_SCALE));
					tree2->AttachTo(RootComponent);
					tree2->RegisterComponent();
					AddFeatureToMap(idx, tree2);
				}

			}

			// Flags
			if (planet->Landblocks[idx].OwnerCiv > -1) {
				FVector loc = FVector(MX * 200.0f, MY * 200.0f, MZ);
				UStaticMeshComponent * flag = NewObject<UStaticMeshComponent>(this);
				flag->SetStaticMesh(Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, TEXT("StaticMesh'/Game/WorldMap/Models/wm_flag.wm_flag'"), nullptr, LOAD_None, nullptr)));
				flag->SetRelativeLocation(loc);
				flag->SetRelativeScale3D(FVector(WATER_SCALE, WATER_SCALE, WATER_SCALE));

				UMaterial * Material = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, TEXT("Material'/Game/Models/VoxMat/VoxelMaterial.VoxelMaterial'"), nullptr, LOAD_None, nullptr));
				UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(Material, this);
				DynMaterial->SetVectorParameterValue("TintRGB", planet->civilizations[planet->Landblocks[idx].OwnerCiv].Color);
				flag->SetMaterial(0, DynMaterial);
				flag->MarkRenderStateDirty();
				flag->AttachTo(RootComponent);
				flag->RegisterComponent();

				AddFeatureToMap(idx, flag);
			}

			// Roads
			if (testbit(FeatureMaskBits::ROAD_N, planet->Landblocks[idx].Features)) {
				FVector loc = FVector(MX * 200.0f, MY * 200.0f, MZ);
				FTransform trans = FTransform(FRotator(), loc, FVector(WATER_SCALE, WATER_SCALE, WATER_SCALE));

				UStaticMeshComponent * pool = NewObject<UStaticMeshComponent>(this);
				pool->SetStaticMesh(Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, TEXT("StaticMesh'/Game/WorldMap/Models/wm_road.wm_road'"), nullptr, LOAD_None, nullptr)));
				pool->SetRelativeLocation(loc);
				pool->SetRelativeScale3D(FVector(WATER_SCALE, WATER_SCALE, WATER_SCALE));
				pool->SetWorldRotation(FRotator(0.0f, 90.0f, 0.0f));
				pool->SetMobility(EComponentMobility::Movable);
				pool->AttachTo(RootComponent);
				pool->RegisterComponent();
				AddFeatureToMap(idx, pool);
			}
			if (testbit(FeatureMaskBits::ROAD_S, planet->Landblocks[idx].Features)) {
				FVector loc = FVector(MX * 200.0f, MY * 200.0f, MZ);
				FTransform trans = FTransform(FRotator(), loc, FVector(WATER_SCALE, WATER_SCALE, WATER_SCALE));

				UStaticMeshComponent * pool = NewObject<UStaticMeshComponent>(this);
				pool->SetStaticMesh(Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, TEXT("StaticMesh'/Game/WorldMap/Models/wm_road.wm_road'"), nullptr, LOAD_None, nullptr)));
				pool->SetRelativeLocation(loc);
				pool->SetRelativeScale3D(FVector(WATER_SCALE, WATER_SCALE, WATER_SCALE));
				pool->SetRelativeRotation(FRotator(0.0f, 270.0f, 0.0f));
				pool->SetMobility(EComponentMobility::Movable);
				pool->AttachTo(RootComponent);
				pool->RegisterComponent();
				AddFeatureToMap(idx, pool);
			}
			if (testbit(FeatureMaskBits::ROAD_E, planet->Landblocks[idx].Features)) {
				FVector loc = FVector(MX * 200.0f, MY * 200.0f, MZ);
				FTransform trans = FTransform(FRotator(), loc, FVector(WATER_SCALE, WATER_SCALE, WATER_SCALE));

				UStaticMeshComponent * pool = NewObject<UStaticMeshComponent>(this);
				pool->SetStaticMesh(Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, TEXT("StaticMesh'/Game/WorldMap/Models/wm_road.wm_road'"), nullptr, LOAD_None, nullptr)));
				pool->SetRelativeLocation(loc);
				pool->SetRelativeScale3D(FVector(WATER_SCALE, WATER_SCALE, WATER_SCALE));
				pool->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
				pool->SetMobility(EComponentMobility::Movable);
				pool->AttachTo(RootComponent);
				pool->RegisterComponent();
				AddFeatureToMap(idx, pool);
			}
			if (testbit(FeatureMaskBits::ROAD_W, planet->Landblocks[idx].Features)) {
				FVector loc = FVector(MX * 200.0f, MY * 200.0f, MZ);
				FTransform trans = FTransform(FRotator(), loc, FVector(WATER_SCALE, WATER_SCALE, WATER_SCALE));

				UStaticMeshComponent * pool = NewObject<UStaticMeshComponent>(this);
				pool->SetStaticMesh(Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, TEXT("StaticMesh'/Game/WorldMap/Models/wm_road.wm_road'"), nullptr, LOAD_None, nullptr)));
				pool->SetRelativeLocation(loc);
				pool->SetRelativeScale3D(FVector(WATER_SCALE, WATER_SCALE, WATER_SCALE));
				//pool->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
				pool->SetMobility(EComponentMobility::Movable);
				pool->AttachTo(RootComponent);
				pool->RegisterComponent();
				AddFeatureToMap(idx, pool);
			}
		}
	}
}