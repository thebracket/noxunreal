// (c) 2016 - Present, Bracket Productions

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "NoxGameInstance.generated.h"

class NRaws;
class UNPlanet;
class UNRegion;
class UBECS;

/**
 * 
 */
UCLASS()
class NOXUNREAL_API UNoxGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:

	NRaws * GetRaws();

	UFUNCTION(BlueprintCallable)
	UNPlanet * GetPlanet();

	UFUNCTION(BlueprintCallable)
	UNRegion * GetRegion();

	UFUNCTION(BlueprintCallable)
	UBECS * GetECS();

private:
	NRaws * raws_instance = nullptr;

	UPROPERTY()
	UNPlanet * planet_instance = nullptr;

	UPROPERTY()
	UNRegion * region_instance = nullptr;

	UPROPERTY()
	UBECS * ecs_instance = nullptr;
	
};
