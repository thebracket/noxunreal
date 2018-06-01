// (c) 2016 - Present, Bracket Productions

#pragma once

#include "CoreMinimal.h"
#include "pcg.h"

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
	pcg32_random_t rng;
};

inline float distance2d(const int &x1, const int &y1, const int &x2, const int &y2) noexcept {
	const float dx = (float)x1 - (float)x2;
	const float dy = (float)y1 - (float)y2;
	return FMath::Sqrt((dx*dx) + (dy*dy));
}

inline float distance2d_squared(const int &x1, const int &y1, const int &x2, const int &y2) noexcept {
	const float dx = (float)x1 - (float)x2;
	const float dy = (float)y1 - (float)y2;
	return (dx*dx) + (dy*dy);
}

inline float distance3d(const int &x1, const int &y1, const int &z1, const int &x2, const int &y2, const int &z2) noexcept
{
	const float dx = (float)x1 - (float)x2;
	const float dy = (float)y1 - (float)y2;
	const float dz = (float)z1 - (float)z2;
	return FMath::Sqrt((dx*dx) + (dy*dy) + (dz*dz));
}

inline float distance3d_squared(const int &x1, const int &y1, const int &z1, const int &x2, const int &y2, const int &z2) noexcept
{
	float dx = (float)x1 - (float)x2;
	float dy = (float)y1 - (float)y2;
	float dz = (float)z1 - (float)z2;
	return (dx*dx) + (dy*dy) + (dz*dz);
}

/*
* Perform a function for each line element between x1/y1 and x2/y2. We used to use Bresenham's line,
* but benchmarking showed a simple float-based setup to be faster.
*/
template <typename FUNC>
inline void line_func(const int &x1, const int &y1, const int &x2, const int &y2, FUNC &&func) noexcept
{
	float x = static_cast<float>(x1) + 0.5F;
	float y = static_cast<float>(y1) + 0.5F;
	const float dest_x = static_cast<float>(x2);
	const float dest_y = static_cast<float>(y2);
	const float n_steps = distance2d(x1, y1, x2, y2);
	const int steps = static_cast<const int>(FMath::FloorToInt(n_steps) + 1);
	const float slope_x = (dest_x - x) / n_steps;
	const float slope_y = (dest_y - y) / n_steps;

	for (int i = 0; i < steps; ++i) {
		func(static_cast<int>(x), static_cast<int>(y));
		x += slope_x;
		y += slope_y;
	}
}
