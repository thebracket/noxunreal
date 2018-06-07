// (c) 2016 - Present, Bracket Productions

#pragma once

#include "CoreMinimal.h"

inline void setbit(const uint32 n, uint32 &bits) noexcept { bits |= n; }
inline void resetbit(const uint32 n, uint32 &bits) noexcept { bits &= ~n; }
inline bool testbit(const uint32 n, uint32 &bits) noexcept { return (bits & n) != 0; }
