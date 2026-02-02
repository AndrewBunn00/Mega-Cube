#pragma once

#include <cstdint>
#include <cstring>

// Use real classes from LED Display
#include "power/Color.h"
#include "power/Math3D.h"
#include "power/Math8.h"
#include "power/Noise.h"
#include "power/Particle.h"

// Simulator version of the Display class
// Provides the same interface as the real Display for LED cube
// but renders to an OpenGL window instead

class Display {
public:
    static const int width = 16;
    static const int height = 16;
    static const int depth = 16;

    // Double-buffered cube (same as real Display)
    static Color cube[2][width][height][depth];
    static uint32_t cubeBuffer;

    static void begin();
    static void update();
    static void clear();

    // Get raw buffer for rendering
    static uint8_t* getRawBuffer();

private:
    static uint8_t rawBuffer[16][16][16][3];  // For renderer
};

// Global config stub (simplified for simulator)
struct SimConfig {
    struct {
        uint16_t max_milliamps = 18000;
        float brightness = 1.0f;
    } power;

    struct {
        uint8_t motionBlur = 64;
    } animation;
};

extern SimConfig simConfig;

// Global noise generator (same as real cube)
extern Noise noise;

// Define the center of the physical cube coordinates (matching Graphics.h)
#define CX 7.5f
#define CY 7.5f
#define CZ 7.5f

// Set voxel using physical cube coordinates
inline void voxel(const uint8_t x, const uint8_t y, const uint8_t z, const Color& c) {
    if (((x | y | z) & 0xF0) == 0) {
        Display::cube[Display::cubeBuffer][x][y][z] = c;
    }
}

// Set voxel using system coordinates (Vector3 version)
inline void voxel(const Vector3& v, const Color& c) {
    int16_t x = (int16_t)(v.x + 32768.5f + CX) - 32768;
    int16_t y = (int16_t)(v.y + 32768.5f + CY) - 32768;
    int16_t z = (int16_t)(v.z + 32768.5f + CZ) - 32768;
    if (((x | y | z) & 0xFFF0) == 0) {
        Display::cube[Display::cubeBuffer][x][y][z] = c;
    }
}

// Additive voxel using system coordinates (Vector3 version)
inline void voxel_add(const Vector3& v, const Color& c) {
    int16_t x = (int16_t)(v.x + 32768.5f + CX) - 32768;
    int16_t y = (int16_t)(v.y + 32768.5f + CY) - 32768;
    int16_t z = (int16_t)(v.z + 32768.5f + CZ) - 32768;
    if (((x | y | z) & 0xFFF0) == 0) {
        Display::cube[Display::cubeBuffer][x][y][z] += c;
    }
}

// Radiate - sphere of light with linear falloff (uses maximize)
inline void radiate(const Vector3& v0, const Color& c, const float r) {
    Vector3 v = v0 + Vector3(CX, CY, CZ);
    int16_t x1 = (int16_t)(v.x - r + 1);
    int16_t x2 = (int16_t)(v.x + r);
    int16_t y1 = (int16_t)(v.y - r + 1);
    int16_t y2 = (int16_t)(v.y + r);
    int16_t z1 = (int16_t)(v.z - r + 1);
    int16_t z2 = (int16_t)(v.z + r);

    for (int16_t x = x1; x <= x2; x++)
        for (int16_t y = y1; y <= y2; y++)
            for (int16_t z = z1; z <= z2; z++) {
                if (((x | y | z) & 0xFFF0) == 0) {
                    float radius = (Vector3(x, y, z) - v).magnitude();
                    if (radius < r) {
                        Display::cube[Display::cubeBuffer][x][y][z].maximize(
                            c.scaled(255 * (1 - (radius / r))));
                    }
                }
            }
}

// Radiate5 - sphere of light with r^5 falloff (uses maximize)
inline void radiate5(const Vector3& v0, const Color& c, const float r) {
    Vector3 v = v0 + Vector3(CX, CY, CZ);
    int16_t x1 = (int16_t)(v.x - r + 1);
    int16_t x2 = (int16_t)(v.x + r);
    int16_t y1 = (int16_t)(v.y - r + 1);
    int16_t y2 = (int16_t)(v.y + r);
    int16_t z1 = (int16_t)(v.z - r + 1);
    int16_t z2 = (int16_t)(v.z + r);

    for (int16_t x = x1; x <= x2; x++)
        for (int16_t y = y1; y <= y2; y++)
            for (int16_t z = z1; z <= z2; z++) {
                if (((x | y | z) & 0xFFF0) == 0) {
                    float radius = (Vector3(x, y, z) - v).magnitude();
                    if (radius < r) {
                        Display::cube[Display::cubeBuffer][x][y][z].maximize(
                            c.scaled(255 / (1 + (radius * radius * radius * radius * radius))));
                    }
                }
            }
}

// Graphics namespace for compatibility with demo code
namespace Graphics {
    inline void voxel(uint8_t x, uint8_t y, uint8_t z, const Color& c) {
        ::voxel(x, y, z, c);
    }

    inline void voxel(float x, float y, float z, const Color& c) {
        ::voxel(Vector3(x, y, z), c);
    }

    // Radiate - sphere of light with falloff (legacy interface)
    void radiate(float cx, float cy, float cz, const Color& c, float radius);

    // Line drawing
    void line(float x1, float y1, float z1, float x2, float y2, float z2, const Color& c);
}
