// (c) 2016 - Present, Bracket Productions

#include "RandomNumberGenerator.h"
#include <ctime>

RandomNumberGenerator::RandomNumberGenerator()
{
	pcg32_srandom_r(&rng, time(NULL), (intptr_t)&rng);
}

RandomNumberGenerator::~RandomNumberGenerator()
{
}

RandomNumberGenerator::RandomNumberGenerator(const int &seed) {
	pcg32_srandom_r(&rng, seed, (intptr_t)&rng);
}

int32 RandomNumberGenerator::RollDice(const int32 &n, const int32 &d) {
	int32 total = 0;
	for (int32 i = 0; i < n; ++i) {
		total += pcg32_boundedrand_r(&rng, d) + 1;
	}
	return total;
}

void RandomNumberGenerator::ReSeed(const int32 &seed) {
	pcg32_srandom_r(&rng, seed, (intptr_t)&rng);
}