// (c) 2016 - Present, Bracket Productions

#include "NStringTable.h"
#include "Runtime/Core/Public/Misc/FileHelper.h"
#include "Runtime/Core/Public/HAL/PlatformFilemanager.h"

NStringTable::NStringTable()
{
}

NStringTable::~NStringTable()
{
}

NStringTable LoadStringTable(const int index, const FString &filename) noexcept {
	NStringTable target;
	target.strings.Empty();

	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*filename))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Could not Find File"));
		return target;
	}

	bool result = FFileHelper::LoadFileToStringArray(target.strings, *filename);
	if (!result) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Could not Read File"));
	}

	return target;
}