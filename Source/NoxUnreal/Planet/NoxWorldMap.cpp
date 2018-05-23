// (c) 2016 - Present, Bracket Productions

#include "NoxWorldMap.h"
#include "../Public/NoxGameInstance.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

// Sets default values
ANoxWorldMap::ANoxWorldMap()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));
	RootComponent = mesh;
	mesh->bUseAsyncCooking = true;
	mesh->bCastHiddenShadow = true;
}

// Called when the game starts or when spawned
void ANoxWorldMap::BeginPlay()
{
	Super::BeginPlay();
	UNoxGameInstance * game = Cast<UNoxGameInstance>(UGameplayStatics::GetGameInstance(this));
	planet = game->GetPlanet();

	TArray<FVector> vertices;
	TArray<int32> Triangles;
	TArray<FVector> normals;
	TArray<FVector2D> UV0;
	TArray<FProcMeshTangent> tangents;
	TArray<FLinearColor> vertexColors;
	int TriangleCounter = 0;

	const float WORLD_SCALE = 50.0f;
	const float Z_SCALE = 5.0f;
	const int w = 1 * WORLD_SCALE;
	const int h = 1 * WORLD_SCALE;
	for (int Y = 0; Y < nfu::WORLD_HEIGHT-1; ++Y) {
		for (int X = 0; X < nfu::WORLD_WIDTH-1; ++X) {
			const auto &lb = planet->Landblocks[planet->idx(X, Y)];
			const auto &lbr = planet->Landblocks[planet->idx(X+1, Y)];
			const auto &lbd = planet->Landblocks[planet->idx(X, Y+1)];
			const auto &lbrd = planet->Landblocks[planet->idx(X + 1, Y + 1)];

			const int x = X * WORLD_SCALE;
			const int y = Y * WORLD_SCALE;
			const int z = lb.height * Z_SCALE;
			const int zr = lbr.height * Z_SCALE;
			const int zd = lbd.height * Z_SCALE;
			const int zrd = lbrd.height * Z_SCALE;

			const float H = z + 2.0f;
			const float H2 = z + 1.0f;
			const float HR = zr + 2.0f;
			const float HD = zd + 2.0f;
			const float HRD = zrd + 2.0f;

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

		}
	}

	mesh->CreateMeshSection_LinearColor(0, vertices, Triangles, normals, UV0, vertexColors, tangents, true);
	mesh->ContainsPhysicsTriMeshData(true);
}

// Called every frame
void ANoxWorldMap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

