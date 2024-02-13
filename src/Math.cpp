#include "Math.h"

#include <glm/glm.hpp>

glm::vec2 randomGradient(int ix, int iy, unsigned seed=6482) {
    const unsigned w = 8 * sizeof(unsigned);
    const unsigned s = w / 2; // rotation width
    // unsigned a = hash(ix + seed), b = hash(iy + hash(seed));
    unsigned a = ix, b = iy;
    a *= 3284157443; 
    b ^= a << s | a >> w - s;
    b *= 1911520717; 
    a ^= b << s | b >> w - s;
    a *= 2048419325;
    float random = a * (3.14159265 / ~(~0u >> 1)); // in [0, 2*Pi]
    glm::vec2 v{std::cos(random), std::sin(random)};
    return v;
}

float dotGridGradient(int ix, int iy, float x, float y, unsigned seed) {
    glm::vec2 gradient = randomGradient(ix, iy, seed);
    glm::vec2 d = {
        x - static_cast<float>(ix), 
        y - static_cast<float>(iy)
    };
    return glm::dot(d, gradient);
}

float math::getPerlinNoise(float x, float y, unsigned int seed) {
    int x0 = static_cast<int>(x); 
    int x1 = x0 + 1;
    int y0 = static_cast<int>(y); 
    int y1 = y0 + 1;

    float sx = x - static_cast<float>(x0);
    float sy = y - static_cast<float>(y0);

    float n0, n1;
    n0 = dotGridGradient(x0, y0, x, y, seed);
    n1 = dotGridGradient(x1, y0, x, y, seed);

    float ix0 = interpolate(n0, n1, sx);

    n0 = dotGridGradient(x0, y1, x, y, seed);
    n1 = dotGridGradient(x1, y1, x, y, seed);
    
    float ix1 = interpolate(n0, n1, sx);

    return interpolate(ix0, ix1, sy);
}