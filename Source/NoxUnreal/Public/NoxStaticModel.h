// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NoxStaticModel.generated.h"

UCLASS()
class NOXUNREAL_API ANoxStaticModel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANoxStaticModel();

	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* StaticMeshComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	int modelId;
	int x, y, z;
	float r = 1.0f, g = 1.0f, b = 1.0f;
	float scaleX = 1.0f;
	float scaleY = 1.0f;
	float scaleZ = 1.0f;
	
	UFUNCTION()
	void onZChange();
};
