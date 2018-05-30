// (c) 2016 - Present, Bracket Productions

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "../Public/NoxGameInstance.h"
#include "GameInputManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FonZChange);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FonCameraMove);

UCLASS()
class NOXUNREAL_API AGameInputManager : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGameInputManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere)
	AActor* CameraOne;

	UPROPERTY(BlueprintReadOnly)
	bool LeftClicked;

	UPROPERTY(BlueprintReadOnly)
	bool RightClicked;

	UPROPERTY(BlueprintAssignable, Category = "CameraControl")
	FonZChange ZLevelChanged;

	UPROPERTY(BlueprintAssignable, Category = "CameraControl")
	FonCameraMove CameraMoved;

	const int WORLD_SCALE = 200;
	int zooming = 0;
	int xMove = 0;
	int yMove = 0;
	int zMove = 0;

	void ZoomIn();
	void ZoomOut();
	void ZoomOff();
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
	void LeftClickOn();
	void LeftClickOff();
	void RightClickOn();
	void RightClickOff();
	
	UNRegion * region;
	UBECS * ecs;

	UFUNCTION(BlueprintCallable)
	void TriggerZLevelChange();

	UFUNCTION(BlueprintCallable)
	void UpdateCamera();
};
