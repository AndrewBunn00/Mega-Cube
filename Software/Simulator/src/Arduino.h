/*
 * Arduino compatibility header for simulator
 *
 * Provides Arduino functions used by the shared LED Display code
 * so it can compile on desktop without modification.
 */

#ifndef ARDUINO_H_SIMULATOR
#define ARDUINO_H_SIMULATOR

#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <chrono>

// Arduino types
typedef uint8_t byte;
typedef bool boolean;

// Time functions
inline unsigned long micros() {
    static auto start = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    return (unsigned long)std::chrono::duration_cast<std::chrono::microseconds>(now - start).count();
}

inline unsigned long millis() {
    return micros() / 1000;
}

// Random functions
inline long random(long max) {
    return std::rand() % max;
}

inline long random(long min, long max) {
    if (min >= max) return min;
    return min + (std::rand() % (max - min));
}

inline float randomf() {
    return (float)std::rand() / (float)RAND_MAX;
}

// Math functions (some platforms need these)
#ifndef PI
#define PI 3.14159265358979323846f
#endif

#ifndef TWO_PI
#define TWO_PI 6.28318530717958647692f
#endif

#ifndef HALF_PI
#define HALF_PI 1.57079632679489661923f
#endif

// Constrain
template<typename T>
inline T constrain(T value, T low, T high) {
    if (value < low) return low;
    if (value > high) return high;
    return value;
}

// Map (Arduino's map function)
inline long map(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Min/Max
template<typename T>
inline T min(T a, T b) { return a < b ? a : b; }

template<typename T>
inline T max(T a, T b) { return a > b ? a : b; }

// Abs
template<typename T>
inline T abs(T x) { return x < 0 ? -x : x; }

// Floor function wrapper
using std::floor;
using std::sqrt;
using std::sin;
using std::cos;
using std::tan;
using std::atan2;
using std::pow;
using std::exp;
using std::fabs;

// PROGMEM (not needed on desktop, data is already in RAM)
#define PROGMEM
#define pgm_read_byte(addr) (*(const uint8_t*)(addr))
#define pgm_read_word(addr) (*(const uint16_t*)(addr))
#define pgm_read_float(addr) (*(const float*)(addr))

// String class stub (if needed)
#include <string>
typedef std::string String;

#endif // ARDUINO_H_SIMULATOR
