#pragma once

#include <stdlib.h>

// defines some useful mathematical utilities

namespace math {
    inline float randf(float a, float b) {
        float r = (rand() % 10000) / 10000.0f;
        return r * (b - a) + a;
    }

    inline float absf(float f) {
        return f < 0 ? -f : f;
    }

    inline float clampf(float f, float a, float b) {
        return f < a ? a : (f > b ? b : f);
    }

    inline float interpolate(float a0, float a1, float w) {
        return (a1 - a0) * w + a0;
    }

    float getPerlinNoise(float x, float y, unsigned int seed);
};