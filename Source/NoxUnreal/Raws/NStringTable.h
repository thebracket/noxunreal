// (c) 2016 - Present, Bracket Productions

#pragma once

#include "CoreMinimal.h"
#include "../BEngine/RandomNumberGenerator.h"

namespace string_tables {

	constexpr int FIRST_NAMES_MALE = 1;
	constexpr int FIRST_NAMES_FEMALE = 2;
	constexpr int LAST_NAMES = 3;
	constexpr int NEW_ARRIVAL_QUIPS = 4;
	constexpr int MENU_SUBTITLES = 5;

}

/**
 * 
 */
class NOXUNREAL_API NStringTable
{
public:
	NStringTable();
	~NStringTable();

	inline FString random_entry(RandomNumberGenerator rng = RandomNumberGenerator()) noexcept {
		const auto position = rng.RollDice(1, static_cast<int>(strings.Num())) - 1;
		return strings[position];
	}

	TArray<FString> strings;
};

NStringTable LoadStringTable(const int index, const FString &filename) noexcept;
