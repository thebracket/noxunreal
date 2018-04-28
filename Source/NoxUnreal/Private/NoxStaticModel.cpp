// Fill out your copyright notice in the Description page of Project Settings.

#include "NoxStaticModel.h"
#include "Runtime/Engine/Classes/Components/StaticMeshComponent.h"
#include "../../ThirdParty/libnox/Includes/libnox.h"

// Sets default values
ANoxStaticModel::ANoxStaticModel()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	RootComponent = StaticMeshComponent;
}

// Called when the game starts or when spawned
void ANoxStaticModel::BeginPlay()
{
	Super::BeginPlay();

	// TODO: Clean this up!
	FString voxAddress;
	switch (modelId) {
	case 1: voxAddress = "StaticMesh'/Game/Models/cryobed32.cryobed32'"; break;
	case 2: voxAddress = "StaticMesh'/Game/Models/floorlight.floorlight'"; break;
	case 3: voxAddress = "StaticMesh'/Game/Models/rtg.rtg'"; break;
	case 4: voxAddress = "StaticMesh'/Game/Models/cabinet.cabinet'"; break;
	case 5: voxAddress = "StaticMesh'/Game/Models/battery.battery'"; break;
	case 6: voxAddress = "StaticMesh'/Game/Models/cordex.cordex'"; break;
	case 7: voxAddress = "StaticMesh'/Game/Models/replicator-small.replicator-small'"; break;
	case 8: voxAddress = "StaticMesh'/Game/Models/solarpanel.solarpanel'"; break;
	case 9: voxAddress = "StaticMesh'/Game/Models/woodlogs.woodlogs'"; break;
	case 10: voxAddress = "StaticMesh'/Game/Models/turret.turret'"; break;
	case 23: voxAddress = "StaticMesh'/Game/Models/stairs-up.stairs-up'"; break;
	case 24: voxAddress = "StaticMesh'/Game/Models/stairs-down.stairs-down'"; break;
	case 25: voxAddress = "StaticMesh'/Game/Models/stairs-updown.stairs-updown'"; break;
	case 35: voxAddress = "StaticMesh'/Game/Models/deer.deer'"; break;
	case 36: voxAddress = "StaticMesh'/Game/Models/horse.horse'"; break;
	case 38: voxAddress = "StaticMesh'/Game/Models/badger.badger'"; break;
	case 49: voxAddress = "StaticMesh'/Game/Models/person_base.person_base'"; break;
	case 50: voxAddress = "StaticMesh'/Game/Models/person_hair_short.person_hair_short'"; break;
	case 51: voxAddress = "StaticMesh'/Game/Models/person_hair_long.person_hair_long'"; break;
	case 52: voxAddress = "StaticMesh'/Game/Models/person_hair_pigtails.person_hair_pigtails'"; break;
	case 53: voxAddress = "StaticMesh'/Game/Models/person_hair_mohawk.person_hair_mohawk'"; break;
	case 54: voxAddress = "StaticMesh'/Game/Models/person_hair_balding.person_hair_balding'"; break;
	case 55: voxAddress = "StaticMesh'/Game/Models/person_hair_triangle.person_hair_triangle'"; break;
	case 56: voxAddress = "StaticMesh'/Game/Models/clothes_batman_hat.clothes_batman_hat'"; break;
	case 57: voxAddress = "StaticMesh'/Game/Models/clothes_police_hat.clothes_police_hat'"; break;
	case 58: voxAddress = "StaticMesh'/Game/Models/clothes_cat_ears.clothes_cat_ears'"; break;
	case 59: voxAddress = "StaticMesh'/Game/Models/clothes_tiara.clothes_tiara'"; break;
	case 60: voxAddress = "StaticMesh'/Game/Models/clothes_formal_shirt.clothes_formal_shirt'"; break;
	case 61: voxAddress = "StaticMesh'/Game/Models/clothes_bathrobe.clothes_bathrobe'"; break;
	case 62: voxAddress = "StaticMesh'/Game/Models/clothes_spandex_shirt.clothes_spandex_shirt'"; break;
	case 63: voxAddress = "StaticMesh'/Game/Models/clothes_sports_shirt.clothes_sports_shirt'"; break;
	case 64: voxAddress = "StaticMesh'/Game/Models/clothes_tshirt.clothes_tshirt'"; break;
	case 65: voxAddress = "StaticMesh'/Game/Models/clothes_cargopants.clothes_cargopants'"; break;
	case 66: voxAddress = "StaticMesh'/Game/Models/clothes_longskirt.clothes_longskirt'"; break;
	case 67: voxAddress = "StaticMesh'/Game/Models/clothes_hawaiian_shorts.clothes_hawaiian_shorts'"; break;
	case 68: voxAddress = "StaticMesh'/Game/Models/clothes_spandex_pants.clothes_spandex_pants'"; break;
	case 69: voxAddress = "StaticMesh'/Game/Models/clothes_miniskirt.clothes_miniskirt'"; break;
	case 70: voxAddress = "StaticMesh'/Game/Models/clothes_shoes.clothes_shoes'"; break;
	case 71: voxAddress = "StaticMesh'/Game/Models/clothes_boots.clothes_boots'"; break;
	case 72: voxAddress = "StaticMesh'/Game/Models/clothes_simple_tunic.clothes_simple_tunic'"; break;
	case 73: voxAddress = "StaticMesh'/Game/Models/clothes_britches_simple.clothes_britches_simple'"; break;
	case 74: voxAddress = "StaticMesh'/Game/Models/clothes_cap_simple.clothes_cap_simple'"; break;
	case 93: voxAddress = "StaticMesh'/Game/Models/dung.dung'"; break;
	case 110: voxAddress = "StaticMesh'/Game/Models/corpse.corpse'"; break;
	case 116: voxAddress = "StaticMesh'/Game/Models/raised_flower_bed.raised_flower_bed'"; break;
	case 127: voxAddress = "StaticMesh'/Game/Models/energydoor-open.energydoor-open'"; break;
	case 128: voxAddress = "StaticMesh'/Game/Models/energydoor-closed.energydoor-closed'"; break;
	default: {
		voxAddress = "StaticMesh'/Game/Models/stairs-up.stairs-up'";
		FString comment = TEXT("Unknown voxel model #: ");
		comment.AppendInt(modelId);
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, comment);
	}
	}

	UStaticMesh* stairs;
	stairs = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *voxAddress, nullptr, LOAD_None, nullptr));
	StaticMeshComponent->SetStaticMesh(stairs);	
	if (r != 1.0f || g != 1.0f || b != 1.0f) {
		UMaterial * Material = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, TEXT("Material'/Game/Models/VoxMat/VoxelMaterial.VoxelMaterial'"), nullptr, LOAD_None, nullptr));
		UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(Material, this);
		DynMaterial->SetVectorParameterValue("TintRGB", FLinearColor(r, g, b));
		StaticMeshComponent->SetMaterial(0, DynMaterial);
		StaticMeshComponent->MarkRenderStateDirty();
	}
}

// Called every frame
void ANoxStaticModel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float cx, cy, cz, zoom;
	bool perspective;
	int mode;
	nf::get_camera_position(cx, cy, cz, zoom, perspective, mode);

	if (z <= cz) {
		StaticMeshComponent->SetVisibility(true);
	}
	else 
	{
		StaticMeshComponent->SetVisibility(false);
	}
}

