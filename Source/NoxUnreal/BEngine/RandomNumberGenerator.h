// (c) 2016 - Present, Bracket Productions

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Core/Public/Math/RandomStream.h"

/**
 * Provides RNG functionality
 */
class NOXUNREAL_API RandomNumberGenerator
{
public:
	RandomNumberGenerator();
	RandomNumberGenerator(const int32 &seed);
	~RandomNumberGenerator();

	int32 RollDice(const int32 &n, const int32 &d);
	void ReSeed(const int32 &seed);

private:
	FRandomStream rng;
};

inline float distance2d(const int &x1, const int &y1, const int &x2, const int &y2) noexcept {
	const float dx = (float)x1 - (float)x2;
	const float dy = (float)y1 - (float)y2;
	return FMath::Sqrt((dx*dx) + (dy*dy));
}
