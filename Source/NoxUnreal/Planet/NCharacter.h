// (c) 2016 - Present, Bracket Productions

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../BEngine/BECS.h"
#include "Runtime/Engine/Classes/Components/TextRenderComponent.h"
#include "NCharacter.generated.h"

UCLASS()
class NOXUNREAL_API ANCharacter : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANCharacter();

	USceneComponent * SceneComp;

	UPROPERTY(BlueprintReadOnly)
	int32 id;

	UPROPERTY(BlueprintReadOnly)
	FString CharacterName;

	UPROPERTY(BlueprintReadOnly)
	FString FirstName;

	UPROPERTY(BlueprintReadOnly)
	TMap<int, UStaticMeshComponent *> components;

	UPROPERTY(BlueprintReadOnly)
	UTextRenderComponent * label;

	UPROPERTY(BlueprintReadOnly)
	UTextRenderComponent * emote = nullptr;

	UPROPERTY(BlueprintReadOnly)
	int emoteTimer = 0;

	UPROPERTY(BlueprintReadOnly)
	int x;

	UPROPERTY(BlueprintReadOnly)
	int y;

	UPROPERTY(BlueprintReadOnly)
	int z;

	UPROPERTY(BlueprintReadOnly)
	int rotation;

	void RefreshModels();
	void AddModel(int modelId, int componentId, FNColor tint);

	void Show(const bool &state);
	void SetEmote(const int &id, const FString &text);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	FString GetModelPath(const int &modelId);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
