// (c) 2016 - Present, Bracket Productions

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Public/NoxGameInstance.h"
#include "NDisplayManager.generated.h"

UCLASS()
class NOXUNREAL_API ANDisplayManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANDisplayManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Reference holders
	NRaws * raws;
	UBECS * ecs;
	UNPlanet * planet;
	UNRegion * region;
	
	// Materials
	UPROPERTY(BlueprintReadOnly)
	TMap<int, FString> MaterialAtlas;

};
