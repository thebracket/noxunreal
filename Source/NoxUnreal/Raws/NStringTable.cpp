// (c) 2016 - Present, Bracket Productions

#include "NStringTable.h"
#include "Runtime/Core/Public/Misc/FileHelper.h"

NStringTable::NStringTable()
{
}

NStringTable::~NStringTable()
{
}

NStringTable LoadStringTable(const int index, const FString &filename) noexcept {
	NStringTable target;

	FFileHelper::LoadFileToStringArray(target.strings, &filename[0]);

	return target;
}