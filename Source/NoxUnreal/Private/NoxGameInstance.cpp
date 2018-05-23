// (c) 2016 - Present, Bracket Productions

#include "NoxGameInstance.h"


NRaws * UNoxGameInstance::GetRaws() {
	if (raws_instance == nullptr) {
		raws_instance = new NRaws();
		raws_instance->LoadRaws();
	}
	return raws_instance;
}

