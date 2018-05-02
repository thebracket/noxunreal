// Fill out your copyright notice in the Description page of Project Settings.

#include "CameraDirector.h"
#include "../../ThirdParty/libnox/Includes/libnox.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "Components/InputComponent.h"

// Sets default values
ACameraDirector::ACameraDirector()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessPlayer = EAutoReceiveInput::Player0;

}

// Called when the game starts or when spawned
void ACameraDirector::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACameraDirector::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAction("ZoomIn", IE_Pressed, this, &ACameraDirector::ZoomIn);
	PlayerInputComponent->BindAction("ZoomOut", IE_Pressed, this, &ACameraDirector::ZoomOut);
	PlayerInputComponent->BindAction("ZoomIn", IE_Released, this, &ACameraDirector::ZoomOff);
	PlayerInputComponent->BindAction("ZoomOut", IE_Released, this, &ACameraDirector::ZoomOff);

	PlayerInputComponent->BindAction("CameraLeft", IE_Pressed, this, &ACameraDirector::CameraLeft);
	PlayerInputComponent->BindAction("CameraRight", IE_Pressed, this, &ACameraDirector::CameraRight);
	PlayerInputComponent->BindAction("CameraNorth", IE_Pressed, this, &ACameraDirector::CameraNorth);
	PlayerInputComponent->BindAction("CameraSouth", IE_Pressed, this, &ACameraDirector::CameraSouth);
	PlayerInputComponent->BindAction("CameraUp", IE_Pressed, this, &ACameraDirector::CameraUp);
	PlayerInputComponent->BindAction("CameraDown", IE_Pressed, this, &ACameraDirector::CameraDown);

	PlayerInputComponent->BindAction("CameraLeft", IE_Released, this, &ACameraDirector::CameraOff);
	PlayerInputComponent->BindAction("CameraRight", IE_Released, this, &ACameraDirector::CameraOff);
	PlayerInputComponent->BindAction("CameraNorth", IE_Released, this, &ACameraDirector::CameraOff);
	PlayerInputComponent->BindAction("CameraSouth", IE_Released, this, &ACameraDirector::CameraOff);

	PlayerInputComponent->BindAction("CameraMode", IE_Pressed, this, &ACameraDirector::CameraMode);
	PlayerInputComponent->BindAction("CameraPerspective", IE_Pressed, this, &ACameraDirector::CameraPerspective);

	PlayerInputComponent->BindAction("PauseToggler", IE_Pressed, this, &ACameraDirector::PauseToggler);
	PlayerInputComponent->BindAction("PauseOneStep", IE_Pressed, this, &ACameraDirector::PauseOneStep);
}

// Called every frame
void ACameraDirector::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Input Support

	if (zooming == 1) {
		nf::camera_zoom_in();
	}
	else if (zooming == 2) {
		nf::camera_zoom_out();
	}

	if (xMove != 0 || yMove != 0 || zMove != 0) {
		nf::camera_move(xMove, yMove, zMove);
		zMove = 0;
	}

	// Render
	float x, y, z, zoom;
	bool perspective;
	int mode;
	nf::get_camera_position(x, y, z, zoom, perspective, mode);

	FVector camera_position;
	FVector camera_target = FVector(x * WORLD_SCALE, y * WORLD_SCALE, z * WORLD_SCALE);

	switch (mode) {
	case 0: camera_position = FVector(x * WORLD_SCALE, (y + (zoom / 3.0f)) * WORLD_SCALE, (z + zoom) * WORLD_SCALE); break;
	case 1: camera_position = FVector(x * WORLD_SCALE, (y + zoom) * WORLD_SCALE, z * WORLD_SCALE); break;
	case 2: camera_position = FVector((x + zoom) * WORLD_SCALE, (y + zoom) * WORLD_SCALE, (z+zoom) * WORLD_SCALE); break;
	case 3: camera_position = FVector((x - zoom) * WORLD_SCALE, (y + zoom) * WORLD_SCALE, (z+zoom) * WORLD_SCALE); break;
	case 4: camera_position = FVector((x + zoom) * WORLD_SCALE, (y + zoom) * WORLD_SCALE, (z-zoom) * WORLD_SCALE); break;
	case 5: camera_position = FVector((x - zoom) * WORLD_SCALE, (y + zoom) * WORLD_SCALE, (z - zoom) * WORLD_SCALE); break;
	}

	FRotator camrot = UKismetMathLibrary::FindLookAtRotation(camera_position, camera_target);

	CameraOne->SetActorLocation(camera_position);
	CameraOne->SetActorRotation(camrot);
}

void ACameraDirector::ZoomIn() {
	zooming = 1;
}

void ACameraDirector::ZoomOut() {
	zooming = 2;
}

void ACameraDirector::ZoomOff() {
	zooming = 0;
}

void ACameraDirector::CameraLeft() {
	xMove = -1;
}

void ACameraDirector::CameraRight() {
	xMove = 1;
}

void ACameraDirector::CameraNorth() {
	yMove = -1;
}

void ACameraDirector::CameraSouth() {
	yMove = 1;
}

void ACameraDirector::CameraUp() {
	zMove = 1;
}

void ACameraDirector::CameraDown() {
	zMove = -1;
}

void ACameraDirector::CameraOff() {
	xMove = 0;
	yMove = 0;
	zMove = 0;
}

void ACameraDirector::CameraMode() {
	nf::toggle_camera_mode();
}

void ACameraDirector::CameraPerspective() {
	nf::toggle_camera_perspective();
}

void ACameraDirector::PauseToggler() {
	int mode = nf::get_pause_mode();
	if (mode == 0) {
		nf::set_pause_mode(1);
	}
	else {
		nf::set_pause_mode(0);
	}
}

void ACameraDirector::PauseOneStep() {
	nf::set_pause_mode(2);
}

