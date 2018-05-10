// (c) 2016 - Present, Bracket Productions

#include "NoxStaticFoliage.h"
#include "Runtime/Engine/Classes/Components/StaticMeshComponent.h"
#include "../../ThirdParty/libnox/Includes/libnox.h"

// Sets default values
ANoxStaticFoliage::ANoxStaticFoliage()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	RootComponent = StaticMeshComponent;
}

// Called when the game starts or when spawned
void ANoxStaticFoliage::BeginPlay()
{
	Super::BeginPlay();
	
	// TODO: Some variety!

	FString voxAddress("StaticMesh'/Game/Foliage/Meshes/Bush/SM_bush1.SM_bush1'");
	UStaticMesh* stairs;
	stairs = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *voxAddress, nullptr, LOAD_None, nullptr));
	StaticMeshComponent->SetStaticMesh(stairs);
	StaticMeshComponent->MarkRenderStateDirty();
}

// Called every frame
void ANoxStaticFoliage::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float cx, cy, cz, zoom;
	bool perspective;
	int mode;
	nf::get_camera_position(cx, cy, cz, zoom, perspective, mode);

	if (z <= cz) {
		StaticMeshComponent->SetVisibility(true);
	}
	else
	{
		StaticMeshComponent->SetVisibility(false);
	}
}

