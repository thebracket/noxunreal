// (c) 2016 - Present, Bracket Productions

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "../Raws/NRaws.h"
#include "../Planet/NPlanet.h"
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

private:
	NRaws * raws_instance = nullptr;

	UPROPERTY()
	UNPlanet * planet_instance = nullptr;
	
};
