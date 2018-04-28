// (c) 2016 - Present, Bracket Productions

#include "NoxStartup.h"
#include "../../ThirdParty/libnox/Includes/libnox.h"

// Sets default values
ANoxStartup::ANoxStartup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ANoxStartup::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANoxStartup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ANoxStartup::LoadAndSetup() {
	nf::setup_raws();
	GameSubtitle = nf::get_game_subtitle();
	WorldExists = nf::is_world_loadable();
}

