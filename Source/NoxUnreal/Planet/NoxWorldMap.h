// (c) 2016 - Present, Bracket Productions

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NPlanet.h"
#include "ProceduralMeshComponent.h"
#include "NoxWorldMap.generated.h"

UCLASS()
class NOXUNREAL_API ANoxWorldMap : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANoxWorldMap();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadOnly)
	UNPlanet * planet = nullptr;

	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent * mesh;
	
};