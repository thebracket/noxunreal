// (c) 2016 - Present, Bracket Productions

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "../Raws/NRaws.h"
#include "../Planet/NPlanet.h"
#include "../Planet/NRegion.h"
#include "../BEngine/BECS.h"
#include "NoxGameInstance.generated.h"

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
