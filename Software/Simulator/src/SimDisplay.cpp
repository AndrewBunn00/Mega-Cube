#include "SimDisplay.h"
#include <cmath>

// Static member definitions
Color Display::cube[2][16][16][16];
uint32_t Display::cubeBuffer = 0;
uint8_t Display::rawBuffer[16][16][16][3];

SimConfig simConfig;

// Global noise generator
Noise noise;

void Display::begin() {
    clear();
    cubeBuffer = 0;
}

void Display::update() {
    // Copy current buffer to raw buffer for rendering
    // Apply motion blur between current and previous frame
    uint8_t blur = simConfig.animation.motionBlur;
    uint32_t prev = 1 - cubeBuffer;

    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
            for (int z = 0; z < 16; z++) {
                // Real Color::blend modifies in place and returns reference
                Color c = cube[cubeBuffer][x][y][z];
                c.blend(blur, cube[prev][x][y][z]);
                rawBuffer[x][y][z][0] = c.r;
                rawBuffer[x][y][z][1] = c.g;
                rawBuffer[x][y][z][2] = c.b;
            }
        }
    }

    // Swap buffers
    cubeBuffer = 1 - cubeBuffer;
    clear();
}

void Display::clear() {
    memset(cube[cubeBuffer], 0, sizeof(cube[cubeBuffer]));
}

uint8_t* Display::getRawBuffer() {
    return &rawBuffer[0][0][0][0];
}

// Graphics namespace implementations
namespace Graphics {

void radiate(float cx, float cy, float cz, const Color& c, float radius) {
    // Delegate to the global radiate function
    ::radiate(Vector3(cx, cy, cz), c, radius);
}

void line(float x1, float y1, float z1, float x2, float y2, float z2, const Color& c) {
    Vector3 a(x1, y1, z1);
    Vector3 b(x2, y2, z2);
    Vector3 n = (a - b);
    float steps = 1 + std::max(std::abs(n.z), std::max(std::abs(n.x), std::abs(n.y)));
    Vector3 inc = n / steps;
    for (int i = 0; i <= (int)steps; i++) {
        ::voxel(a - (inc * i), c);
    }
}

}  // namespace Graphics
