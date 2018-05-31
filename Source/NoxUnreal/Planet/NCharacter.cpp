// (c) 2016 - Present, Bracket Productions

#include "NCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "../Public/NoxGameInstance.h"

// Sets default values
ANCharacter::ANCharacter()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("ActorSceneComponent"));
	RootComponent = SceneComp;
}

// Called when the game starts or when spawned
void ANCharacter::BeginPlay()
{
	Super::BeginPlay();
	FString ActorName = CharacterName;
	ActorName.AppendInt(id);
	SetActorLabel(ActorName);
	RefreshModels();
}

void ANCharacter::AddModel(int modelId, int componentId, FNColor tint) {
	components.Add(componentId, nullptr);
	FString voxAddress = GetModelPath(modelId);
	FString LayerName = TEXT("Layer_");
	LayerName.AppendInt(componentId);
	components[componentId] = NewObject<UStaticMeshComponent>(this, *LayerName);
	UStaticMesh* theModel;
	theModel = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *voxAddress, nullptr, LOAD_None, nullptr));
	components[componentId]->SetStaticMesh(theModel);

	UMaterial * Material = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, TEXT("Material'/Game/Models/VoxMat/VoxelMaterial.VoxelMaterial'"), nullptr, LOAD_None, nullptr));
	UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(Material, this);
	DynMaterial->SetVectorParameterValue("TintRGB", FLinearColor(tint.r, tint.g, tint.b));
	components[componentId]->SetMaterial(0, DynMaterial);
	components[componentId]->MarkRenderStateDirty();

	components[componentId]->AttachTo(RootComponent);
	components[componentId]->RegisterComponent();
}

void ANCharacter::RefreshModels() {
	// TODO: Clear
	UNoxGameInstance * game = Cast<UNoxGameInstance>(UGameplayStatics::GetGameInstance(this));
	auto ecs = game->GetECS();
	auto species = ecs->ecs.GetComponent<species_t>(id);

	// Add model 49 - base
	AddModel(49, -2, species->skin_color.Value);

	// Add hair
	int hair_vox;
	switch (species->hair_style) {
	case SHORT_HAIR: hair_vox = 50; break;
	case LONG_HAIR: hair_vox = 51; break;
	case PIGTAILS: hair_vox = 52; break;
	case MOHAWK: hair_vox = 53; break;
	case BALDING: hair_vox = 54; break;
	case TRIANGLE: hair_vox = 55; break;
	default: hair_vox = 0;
	}
	if (hair_vox > 0) AddModel(hair_vox, -1, species->hair_color.Value);

	// Add items
	ecs->ecs.Each<item_t, item_carried_t>([this](int &item_id, item_t &item, item_carried_t &carried) {
		if (carried.carried_by == id && item.clothing_layer > 0) {
			AddModel(item.clothing_layer, item_id, item.clothing_color);
		}
	});
}

// Called every frame
void ANCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FString ANCharacter::GetModelPath(const int &modelId) {
	FString voxAddress;
	switch (modelId) {
	case -1: voxAddress = "StaticMesh'/Game/Foliage/Meshes/Trees/SM_tree1.SM_tree1'"; break;
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
	case 11: voxAddress = "StaticMesh'/Game/Models/simpleaxe.simpleaxe'"; break;
	case 12: voxAddress = "StaticMesh'/Game/Models/crate-fakefire.crate-fakefire'"; break;
	case 13: voxAddress = "StaticMesh'/Game/Models/crate-tent.crate-tent'"; break;
	case 14: voxAddress = "StaticMesh'/Game/Models/campfire-fake.campfire-fake'"; break;
	case 15: voxAddress = "StaticMesh'/Game/Models/tent.tent'"; break;
	case 16: voxAddress = "StaticMesh'/Game/Models/sawmill.sawmill'"; break;
	case 17: voxAddress = "StaticMesh'/Game/Models/block.block'"; break;
	case 18: voxAddress = "StaticMesh'/Game/Models/wastewood.wastewood'"; break;
	case 19: voxAddress = "StaticMesh'/Game/Models/bonecarver.bonecarver'"; break;
	case 20: voxAddress = "StaticMesh'/Game/Models/butcher.butcher'"; break;
	case 21: voxAddress = "StaticMesh'/Game/Models/carpenter.carpenter'"; break;
	case 22: voxAddress = "StaticMesh'/Game/Models/chemist.chemist'"; break;
	case 23: voxAddress = "StaticMesh'/Game/Models/stairs-up.stairs-up'"; break;
	case 24: voxAddress = "StaticMesh'/Game/Models/stairs-down.stairs-down'"; break;
	case 25: voxAddress = "StaticMesh'/Game/Models/stairs-updown.stairs-updown'"; break;
	case 26: voxAddress = "StaticMesh'/Game/Models/tailor.tailor'"; break;
	case 27: voxAddress = "StaticMesh'/Game/Models/forge.forge'"; break;
	case 28: voxAddress = "StaticMesh'/Game/Models/simplepick.simplepick'"; break;
	case 29: voxAddress = "StaticMesh'/Game/Models/boulder.boulder'"; break;
	case 30: voxAddress = "StaticMesh'/Game/Models/ore.ore'"; break;
	case 31: voxAddress = "StaticMesh'/Game/Models/powder.powder'"; break;
	case 32: voxAddress = "StaticMesh'/Game/Models/oil.oil'"; break;
	case 33: voxAddress = "StaticMesh'/Game/Models/stonecutter.stonecutter'"; break;
	case 34: voxAddress = "StaticMesh'/Game/Models/smelter.smelter'"; break;
	case 35: voxAddress = "StaticMesh'/Game/Models/deer.deer'"; break;
	case 36: voxAddress = "StaticMesh'/Game/Models/horse.horse'"; break;
	case 37: voxAddress = "StaticMesh'/Game/Models/armadillo.armadillo'"; break;
	case 38: voxAddress = "StaticMesh'/Game/Models/badger.badger'"; break;
	case 39: voxAddress = "StaticMesh'/Game/Models/badger.badger'"; break;
	case 40: voxAddress = "StaticMesh'/Game/Models/charcoal_hut.charcoal_hut'"; break;
	case 41: voxAddress = "StaticMesh'/Game/Models/still.still'"; break;
	case 42: voxAddress = "StaticMesh'/Game/Models/loom.loom'"; break;
	case 43: voxAddress = "StaticMesh'/Game/Models/glass_furnace.glass_furnace'"; break;
	case 44: voxAddress = "StaticMesh'/Game/Models/mason.mason'"; break;
	case 45: voxAddress = "StaticMesh'/Game/Models/mechanic.mechanic'"; break;
	case 46: voxAddress = "StaticMesh'/Game/Models/tanner.tanner'"; break;
	case 47: voxAddress = "StaticMesh'/Game/Models/charcoal.charcoal'"; break;
	case 48: voxAddress = "StaticMesh'/Game/Models/ant.ant'"; break;
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
	case 75: voxAddress = "StaticMesh'/Game/Models/ground_helmet.ground_helmet'"; break;
	case 76: voxAddress = "StaticMesh'/Game/Models/ground_shirt.ground_shirt'"; break;
	case 77: voxAddress = "StaticMesh'/Game/Models/ground_pants.ground_pants'"; break;
	case 78: voxAddress = "StaticMesh'/Game/Models/ground_shoes.ground_shoes'"; break;
	case 79: voxAddress = "StaticMesh'/Game/Models/meat.meat'"; break;
	case 80: voxAddress = "StaticMesh'/Game/Models/foodplate.foodplate'"; break;
	case 81: voxAddress = "StaticMesh'/Game/Models/liquid.liquid'"; break;
	case 82: voxAddress = "StaticMesh'/Game/Models/veggie.veggie'"; break;
	case 83: voxAddress = "StaticMesh'/Game/Models/table.table'"; break;
	case 84: voxAddress = "StaticMesh'/Game/Models/chair.chair'"; break;
	case 85: voxAddress = "StaticMesh'/Game/Models/door_ground.door_ground'"; break;
	case 86: voxAddress = "StaticMesh'/Game/Models/bed.bed'"; break;
	case 87: voxAddress = "StaticMesh'/Game/Models/axeblade.axeblade'"; break;
	case 88: voxAddress = "StaticMesh'/Game/Models/lathe.lathe'"; break;
	case 89: voxAddress = "StaticMesh'/Game/Models/hay.hay'"; break;
	case 90: voxAddress = "StaticMesh'/Game/Models/thread.thread'"; break;
	case 91: voxAddress = "StaticMesh'/Game/Models/cloth.cloth'"; break;
	case 92: voxAddress = "StaticMesh'/Game/Models/paper.paper'"; break;
	case 93: voxAddress = "StaticMesh'/Game/Models/dung.dung'"; break;
	case 94: voxAddress = "StaticMesh'/Game/Models/mechanism.mechanism'"; break;
	case 95: voxAddress = "StaticMesh'/Game/Models/circuit.circuit'"; break;
	case 96: voxAddress = "StaticMesh'/Game/Models/bone.bone'"; break;
	case 97: voxAddress = "StaticMesh'/Game/Models/hide.hide'"; break;
	case 98: voxAddress = "StaticMesh'/Game/Models/skull.skull'"; break;
	case 99: voxAddress = "StaticMesh'/Game/Models/cage.cage'"; break;
	case 100: voxAddress = "StaticMesh'/Game/Models/club.club'"; break;
	case 101: voxAddress = "StaticMesh'/Game/Models/club_spiked.club_spiked'"; break;
	case 102: voxAddress = "StaticMesh'/Game/Models/pointystick.pointystick'"; break;
	case 103: voxAddress = "StaticMesh'/Game/Models/hammer.hammer'"; break;
	case 104: voxAddress = "StaticMesh'/Game/Models/sword.sword'"; break;
	case 105: voxAddress = "StaticMesh'/Game/Models/knife.knife'"; break;
	case 106: voxAddress = "StaticMesh'/Game/Models/atlatl.atlatl'"; break;
	case 107: voxAddress = "StaticMesh'/Game/Models/bow.bow'"; break;
	case 108: voxAddress = "StaticMesh'/Game/Models/arrow.arrow'"; break;
	case 109: voxAddress = "StaticMesh'/Game/Models/crossbow.crossbow'"; break;
	case 110: voxAddress = "StaticMesh'/Game/Models/corpse.corpse'"; break;
	case 111: voxAddress = "StaticMesh'/Game/Models/simplepick_held.simplepick_held'"; break;
	case 112: voxAddress = "StaticMesh'/Game/Models/simpleaxe_held.simpleaxe_held'"; break;
	case 113: voxAddress = "StaticMesh'/Game/Models/hoe.hoe'"; break;
	case 114: voxAddress = "StaticMesh'/Game/Models/dung_heap.dung_heap'"; break;
	case 115: voxAddress = "StaticMesh'/Game/Models/seed.seed'"; break;
	case 116: voxAddress = "StaticMesh'/Game/Models/raised_flower_bed.raised_flower_bed'"; break;
	case 117: voxAddress = "StaticMesh'/Game/Models/wall.wall'"; break;
	case 118: voxAddress = "StaticMesh'/Game/Models/floor.floor'"; break;
	case 119: voxAddress = "StaticMesh'/Game/Models/ramp.ramp'"; break;
	case 120: voxAddress = "StaticMesh'/Game/Models/carpenter_intermediate.carpenter_intermediate'"; break;
	case 121: voxAddress = "StaticMesh'/Game/Models/nitrogen_extractor.nitrogen_extractor'"; break;
	case 122: voxAddress = "StaticMesh'/Game/Models/leatherworker.leatherworker'"; break;
	case 123: voxAddress = "StaticMesh'/Game/Models/refinery.refinery'"; break;
	case 124: voxAddress = "StaticMesh'/Game/Models/lever-on.lever-on'"; break;
	case 125: voxAddress = "StaticMesh'/Game/Models/lever-off.lever-off'"; break;
	case 126: voxAddress = "StaticMesh'/Game/Models/pressureplate.pressureplate'"; break;
	case 127: voxAddress = "StaticMesh'/Game/Models/energydoor-open.energydoor-open'"; break;
	case 128: voxAddress = "StaticMesh'/Game/Models/energydoor-closed.energydoor-closed'"; break;
	case 129: voxAddress = "StaticMesh'/Game/Models/spikes-retracted.spikes-retracted'"; break;
	case 130: voxAddress = "StaticMesh'/Game/Models/spikes-extended.spikes-extended'"; break;
	case 131: voxAddress = "StaticMesh'/Game/Models/blades-retracted.blades-retracted'"; break;
	case 132: voxAddress = "StaticMesh'/Game/Models/blades-extended.blades-extended'"; break;
	case 133: voxAddress = "StaticMesh'/Game/Models/stonefall.stonefall'"; break;
	case 134: voxAddress = "StaticMesh'/Game/Models/door_open.door_open'"; break;
	case 135: voxAddress = "StaticMesh'/Game/Models/floodgate_open.floodgate_open'"; break;
	case 136: voxAddress = "StaticMesh'/Game/Models/floodgate_closed.floodgate_closed'"; break;
	case 137: voxAddress = "StaticMesh'/Game/Models/floodgate_ground.floodgate_ground'"; break;
	case 138: voxAddress = "StaticMesh'/Game/Models/float_gauge.float_gauge'"; break;
	case 139: voxAddress = "StaticMesh'/Game/Models/proximity_sensor.proximity_sensor'"; break;
	case 140: voxAddress = "StaticMesh'/Game/Models/oscillator.oscillator'"; break;
	case 141: voxAddress = "StaticMesh'/Game/Models/gate_and.gate_and'"; break;
	case 142: voxAddress = "StaticMesh'/Game/Models/gate_or.gate_or'"; break;
	case 143: voxAddress = "StaticMesh'/Game/Models/gate_not.gate_not'"; break;
	case 144: voxAddress = "StaticMesh'/Game/Models/silicon_refinery.silicon_refinery'"; break;
	case 145: voxAddress = "StaticMesh'/Game/Models/gate_nand.gate_nand'"; break;
	case 146: voxAddress = "StaticMesh'/Game/Models/gate_nor.gate_nor'"; break;
	case 147: voxAddress = "StaticMesh'/Game/Models/gate_xor.gate_xor'"; break;
	case 148: voxAddress = "StaticMesh'/Game/Models/support.support'"; break;
	default: {
		voxAddress = "StaticMesh'/Game/Models/stairs-up.stairs-up'";
		FString comment = TEXT("Unknown voxel model #: ");
		comment.AppendInt(modelId);
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, comment);
	}
	}

	return voxAddress;
}

void ANCharacter::Show(const bool &state) {
	SceneComp->SetVisibility(state);
	for (auto &cr : components) {
		cr.Value->SetVisibility(state);
	}
}