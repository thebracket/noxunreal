// (c) 2016 - Present, Bracket Productions

#include "NDisplayManager.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ANDisplayManager::ANDisplayManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

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

	// Init/Setup Chunks
	region->InitializeChunks();
	region->UpdateChunks();

	// Spawn Chunks
}

// Called every frame
void ANDisplayManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

