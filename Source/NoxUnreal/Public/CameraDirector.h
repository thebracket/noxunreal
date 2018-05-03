// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ProceduralMeshComponent.h"
#include "NoxRegionLayer.h"
#include "CameraDirector.generated.h"

UCLASS()
class NOXUNREAL_API ACameraDirector : public APawn
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACameraDirector();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
	AActor* CameraOne;

	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent * mesh;

	const int WORLD_SCALE = 200;
	
	int zooming = 0;
	void ZoomIn();
	void ZoomOut();
	void ZoomOff();

	int xMove = 0;
	int yMove = 0;
	int zMove = 0;
	void CameraLeft();
	void CameraRight();
	void CameraNorth();
	void CameraSouth();
	void CameraUp();
	void CameraDown();
	void CameraOff();
	void CameraMode();
	void CameraPerspective();
	void PauseToggler();
	void PauseOneStep();

	// Cursor handling
	TMap<int, geometry_chunk> geometry_by_material;
	void Cursors();
};
