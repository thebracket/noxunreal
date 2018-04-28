// (c) 2016 - Present, Bracket Productions

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NoxStartup.generated.h"

UCLASS()
class NOXUNREAL_API ANoxStartup : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANoxStartup();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void LoadAndSetup();
	
	UPROPERTY(BlueprintReadOnly)
	FString GameSubtitle;

	UPROPERTY(BlueprintReadOnly)
	bool WorldExists;
};
