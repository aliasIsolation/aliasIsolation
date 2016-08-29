#pragma once

#include "common.h"


inline float hammersleySample(uint bits, uint seed)
{
    bits = ( bits << 16u) | ( bits >> 16u);
    bits = ((bits & 0x00ff00ffu) << 8u) | ((bits & 0xff00ff00u) >> 8u);
    bits = ((bits & 0x0f0f0f0fu) << 4u) | ((bits & 0xf0f0f0f0u) >> 4u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xccccccccu) >> 2u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xaaaaaaaau) >> 1u);
    bits ^= seed;
    return float(bits) * 2.3283064365386963e-10f; // divide by 1<<32
}

inline glm::vec2 hammersley2d(uint i, uint N, uint seed) {
    return glm::vec2(float(float(i) + 0.5f)/float(N), hammersleySample(i, seed));
}
