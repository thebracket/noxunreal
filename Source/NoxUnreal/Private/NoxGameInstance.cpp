// (c) 2016 - Present, Bracket Productions

#include "NoxGameInstance.h"


NRaws * UNoxGameInstance::GetRaws() {
	if (raws_instance == nullptr) {
		raws_instance = new NRaws();
		raws_instance->LoadRaws();
	}
	return raws_instance;
}

UNPlanet * UNoxGameInstance::GetPlanet() {
	if (planet_instance == nullptr) {
		planet_instance = NewObject<UNPlanet>(this);
	}
	return planet_instance;
}

UNRegion * UNoxGameInstance::GetRegion() {
	if (region_instance == nullptr) {
		region_instance = NewObject<UNRegion>(this);
	}
	return region_instance;
}