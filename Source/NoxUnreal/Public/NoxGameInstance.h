// (c) 2016 - Present, Bracket Productions

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "../Raws/NRaws.h"
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

private:
	NRaws * raws_instance = nullptr;
	
};
