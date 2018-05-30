// (c) 2016 - Present, Bracket Productions

#include "RandomNumberGenerator.h"

RandomNumberGenerator::RandomNumberGenerator()
{
}

RandomNumberGenerator::~RandomNumberGenerator()
{
}

RandomNumberGenerator::RandomNumberGenerator(const int &seed) {
	rng = FRandomStream(seed);
	rng.Initialize(seed);
}

int32 RandomNumberGenerator::RollDice(const int32 &n, const int32 &d) {
	int32 total = 0;

	for (int32 i = 0; i < n; ++i) {
		total += rng.RandRange(1, d);
	}

	return total;
}

void RandomNumberGenerator::ReSeed(const int32 &seed) {
	rng = FRandomStream(seed);
	rng.Initialize(seed);
}