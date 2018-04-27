// Fill out your copyright notice in the Description page of Project Settings.

#include "NoxDynamicLight.h"


// Sets default values
ANoxDynamicLight::ANoxDynamicLight()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	pointlight = CreateDefaultSubobject<UPointLightComponent>(TEXT("Pointlight"));
	RootComponent = pointlight;
}

// Called when the game starts or when spawned
void ANoxDynamicLight::BeginPlay()
{
	Super::BeginPlay();
	pointlight->SetLightColor(FLinearColor(r, g, b));
	pointlight->SetActive(true);
	pointlight->SetCastShadows(true);
	pointlight->SetMobility(EComponentMobility::Movable);
	pointlight->SetIntensity(50000.0f);
}

// Called every frame
void ANoxDynamicLight::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

