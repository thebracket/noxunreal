// (c) 2016 - Present, Bracket Productions

#include "GameInputManager.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "Runtime/Engine/Classes/Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "../BEngine/BECS.h"
#include "../Raws/NRaws.h"
#include "constants.h"

// Sets default values
AGameInputManager::AGameInputManager()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AutoPossessPlayer = EAutoReceiveInput::Player0;

	LeftClicked = false;
	RightClicked = false;
}

// Called when the game starts or when spawned
void AGameInputManager::BeginPlay()
{
	Super::BeginPlay();
	
	UNoxGameInstance * game = Cast<UNoxGameInstance>(UGameplayStatics::GetGameInstance(this));
	region = game->GetRegion();
	ecs = game->GetECS();
	ecs->LinkMasters(game->GetPlanet(), region, game->GetRaws());

	CameraMoved.AddDynamic(this, &AGameInputManager::UpdateCamera);

	CameraMoved.Broadcast();
	ZLevelChanged.Broadcast();
}

// Called every frame
void AGameInputManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (zMove != 0 || xMove != 0 || yMove != 0) CameraMoved.Broadcast();
}

// Called to bind functionality to input
void AGameInputManager::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAction("ZoomIn", IE_Pressed, this, &AGameInputManager::ZoomIn);
	PlayerInputComponent->BindAction("ZoomOut", IE_Pressed, this, &AGameInputManager::ZoomOut);
	PlayerInputComponent->BindAction("ZoomIn", IE_Released, this, &AGameInputManager::ZoomOff);
	PlayerInputComponent->BindAction("ZoomOut", IE_Released, this, &AGameInputManager::ZoomOff);

	PlayerInputComponent->BindAction("CameraLeft", IE_Pressed, this, &AGameInputManager::CameraLeft);
	PlayerInputComponent->BindAction("CameraRight", IE_Pressed, this, &AGameInputManager::CameraRight);
	PlayerInputComponent->BindAction("CameraNorth", IE_Pressed, this, &AGameInputManager::CameraNorth);
	PlayerInputComponent->BindAction("CameraSouth", IE_Pressed, this, &AGameInputManager::CameraSouth);
	PlayerInputComponent->BindAction("CameraUp", IE_Pressed, this, &AGameInputManager::CameraUp);
	PlayerInputComponent->BindAction("CameraDown", IE_Pressed, this, &AGameInputManager::CameraDown);

	PlayerInputComponent->BindAction("CameraLeft", IE_Released, this, &AGameInputManager::CameraOff);
	PlayerInputComponent->BindAction("CameraRight", IE_Released, this, &AGameInputManager::CameraOff);
	PlayerInputComponent->BindAction("CameraNorth", IE_Released, this, &AGameInputManager::CameraOff);
	PlayerInputComponent->BindAction("CameraSouth", IE_Released, this, &AGameInputManager::CameraOff);

	PlayerInputComponent->BindAction("CameraMode", IE_Pressed, this, &AGameInputManager::CameraMode);
	PlayerInputComponent->BindAction("CameraPerspective", IE_Pressed, this, &AGameInputManager::CameraPerspective);

	PlayerInputComponent->BindAction("PauseToggler", IE_Pressed, this, &AGameInputManager::PauseToggler);
	PlayerInputComponent->BindAction("PauseOneStep", IE_Pressed, this, &AGameInputManager::PauseOneStep);

	PlayerInputComponent->BindAction("MouseLeftClick", IE_Pressed, this, &AGameInputManager::LeftClickOn);
	PlayerInputComponent->BindAction("MouseLeftClick", IE_Released, this, &AGameInputManager::LeftClickOff);
	PlayerInputComponent->BindAction("MouseRightClick", IE_Pressed, this, &AGameInputManager::RightClickOn);
	PlayerInputComponent->BindAction("MouseRightClick", IE_Released, this, &AGameInputManager::RightClickOff);
}

void AGameInputManager::ZoomIn() {
	zooming = 1;
	CameraMoved.Broadcast();
}

void AGameInputManager::ZoomOut() {
	zooming = 2;
	CameraMoved.Broadcast();
}

void AGameInputManager::ZoomOff() {
	zooming = 0;
	CameraMoved.Broadcast();
}

void AGameInputManager::CameraLeft() {
	xMove = -1;
	CameraMoved.Broadcast();
}

void AGameInputManager::CameraRight() {
	xMove = 1;
	CameraMoved.Broadcast();
}

void AGameInputManager::CameraNorth() {
	yMove = -1;
	CameraMoved.Broadcast();
}

void AGameInputManager::CameraSouth() {
	yMove = 1;
	CameraMoved.Broadcast();
}

void AGameInputManager::CameraUp() {
	zMove = 1;
	CameraMoved.Broadcast();
}

void AGameInputManager::CameraDown() {
	zMove = -1;
	CameraMoved.Broadcast();
}

void AGameInputManager::CameraOff() {
	xMove = 0;
	yMove = 0;
	zMove = 0;
}

void AGameInputManager::CameraMode() {
	switch (ecs->Camera->camera_mode) {
	case game_camera_mode_t::DIAGONAL_LOOK_NW: {
		ecs->Camera->camera_mode = game_camera_mode_t::DIAGONAL_LOOK_NE;
	}break;
	case game_camera_mode_t::DIAGONAL_LOOK_NE: {
		ecs->Camera->camera_mode = game_camera_mode_t::DIAGONAL_LOOK_SW;
	}break;
	case game_camera_mode_t::DIAGONAL_LOOK_SW: {
		ecs->Camera->camera_mode = game_camera_mode_t::DIAGONAL_LOOK_SE;
	}break;
	case game_camera_mode_t::DIAGONAL_LOOK_SE: {
		ecs->Camera->camera_mode = game_camera_mode_t::FRONT;
	}break;
	case game_camera_mode_t::FRONT: {
		ecs->Camera->camera_mode = game_camera_mode_t::TOP_DOWN;
	} break;
	case game_camera_mode_t::TOP_DOWN: {
		ecs->Camera->camera_mode = game_camera_mode_t::DIAGONAL_LOOK_NW;
	} break;
	default: {} break; // We're ignoring the FPS modes
	}
	CameraMoved.Broadcast();
}

void AGameInputManager::CameraPerspective() {
	ecs->Camera->perspective = !ecs->Camera->perspective;
	CameraMoved.Broadcast();
}

void AGameInputManager::PauseToggler() {
	/*
	int mode = nf::get_pause_mode();
	if (mode == 0) {
		nf::set_pause_mode(1);
	}
	else {
		nf::set_pause_mode(0);
	}
	*/
}

void AGameInputManager::PauseOneStep() {
	//nf::set_pause_mode(2);
}

void AGameInputManager::LeftClickOn() {
	LeftClicked = true;
}

void AGameInputManager::LeftClickOff() {
	LeftClicked = false;
}

void AGameInputManager::RightClickOn() {
	RightClicked = true;
}

void AGameInputManager::RightClickOff() {
	RightClicked = false;
}

void AGameInputManager::TriggerZLevelChange() {
	ZLevelChanged.Broadcast();
}

void AGameInputManager::UpdateCamera() {
	using namespace nfu;

	// Input Support
	if (zooming == 1) {
		--ecs->Camera->zoom_level;
		if (ecs->Camera->zoom_level < 1) ecs->Camera->zoom_level = 1;
		ZLevelChanged.Broadcast();
	}
	else if (zooming == 2) {
		++ecs->Camera->zoom_level;
		if (ecs->Camera->zoom_level > 150) ecs->Camera->zoom_level = 150;
		ZLevelChanged.Broadcast();
	}

	if (xMove != 0 || yMove != 0 || zMove != 0) {
		ecs->CameraPosition->region_x += xMove;
		ecs->CameraPosition->region_y += yMove;
		ecs->CameraPosition->region_z += zMove;

		if (ecs->CameraPosition->region_x < 1) ecs->CameraPosition->region_x = 1;
		if (ecs->CameraPosition->region_x > REGION_WIDTH - 1) ecs->CameraPosition->region_x = REGION_WIDTH - 1;
		if (ecs->CameraPosition->region_y < 1) ecs->CameraPosition->region_y = 1;
		if (ecs->CameraPosition->region_y > REGION_HEIGHT - 1) ecs->CameraPosition->region_y = REGION_HEIGHT - 1;
		if (ecs->CameraPosition->region_z < 1) ecs->CameraPosition->region_z = 1;
		if (ecs->CameraPosition->region_z > REGION_DEPTH - 1) ecs->CameraPosition->region_z = REGION_DEPTH - 1;

		if (zMove != 0) {
			ZLevelChanged.Broadcast();
		}
		zMove = 0;
	}

	// Render
	float x = ecs->CameraPosition->region_x;
	float y = ecs->CameraPosition->region_y;
	float z = ecs->CameraPosition->region_z;
	bool perspective = ecs->Camera->perspective;
	int mode = ecs->Camera->camera_mode;
	float zoom = ecs->Camera->zoom_level;

	FVector camera_position;
	FVector camera_target = FVector(x * WORLD_SCALE, y * WORLD_SCALE, z * WORLD_SCALE);

	switch (mode) {
	case 0: camera_position = FVector(x * WORLD_SCALE, (y + (zoom / 3.0f)) * WORLD_SCALE, (z + zoom) * WORLD_SCALE); break;
	case 1: camera_position = FVector(x * WORLD_SCALE, y * WORLD_SCALE, (z + zoom) * WORLD_SCALE); break;
	case 2: camera_position = FVector((x + zoom) * WORLD_SCALE, (y + zoom) * WORLD_SCALE, (z + zoom) * WORLD_SCALE); break;
	case 3: camera_position = FVector((x - zoom) * WORLD_SCALE, (y + zoom) * WORLD_SCALE, (z + zoom) * WORLD_SCALE); break;
	case 4: camera_position = FVector((x + zoom) * WORLD_SCALE, (y - zoom) * WORLD_SCALE, (z + zoom) * WORLD_SCALE); break;
	case 5: camera_position = FVector((x - zoom) * WORLD_SCALE, (y - zoom) * WORLD_SCALE, (z + zoom) * WORLD_SCALE); break;
	}

	FRotator camrot = UKismetMathLibrary::FindLookAtRotation(camera_position, camera_target);

	CameraOne->SetActorLocation(camera_position);
	CameraOne->SetActorRotation(camrot);

	TArray<UActorComponent *> children = CameraOne->GetComponentsByClass(UCameraComponent::StaticClass());
	for (int32 i = 0; i < children.Num(); ++i) {
		Cast<UCameraComponent>(children[i])->SetOrthoWidth(zoom * 1000.0f);
	}
}
