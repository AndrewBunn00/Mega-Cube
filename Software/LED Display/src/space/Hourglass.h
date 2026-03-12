#ifndef HOURGLASS_H
#define HOURGLASS_H

#include "Animation.h"

class Hourglass : public Animation {
 private:
  static const int MAX_SAND = 300;
  struct Sand {
    float x, y, z;
    bool falling;
  };
  Sand grains[MAX_SAND];
  int numGrains;
  float flipTimer;
  float phase;
  bool flipped;

  static constexpr auto &settings = config.animation.hourglass;

  void resetSand() {
    numGrains = MAX_SAND;
    flipped = false;
    flipTimer = 0;
    phase = 0;

    // Fill top half with sand grains
    for (int i = 0; i < numGrains; i++) {
      // Place grains in the top bulb (y > 0)
      float angle = noise.nextRandom(0, 2.0f * PI);
      float r = noise.nextRandom(0, 1.0f);
      float height = noise.nextRandom(1.0f, 6.5f);
      // Hourglass shape: radius decreases near center
      float maxR = height * 0.8f;
      if (maxR > 5.0f) maxR = 5.0f;
      r *= maxR;
      grains[i].x = cosf(angle) * r;
      grains[i].y = height;
      grains[i].z = sinf(angle) * r;
      grains[i].falling = false;
    }
  }

  float hourglassRadius(float y) {
    // Hourglass shape: wide at top/bottom, narrow at center
    float absY = fabsf(y);
    if (absY < 0.5f) return 0.6f;  // Narrow neck
    return 0.6f + (absY - 0.5f) * 0.85f;
  }

 public:
  void init() {
    state = state_t::STARTING;
    timer_starting = settings.starttime;
    timer_running = settings.runtime;
    timer_ending = settings.endtime;
    resetSand();
  }

  void draw(float dt) {
    setMotionBlur(settings.motionBlur);
    uint8_t brightness = settings.brightness * getBrightness();
    if (state == state_t::STARTING) {
      if (timer_starting.update()) { state = state_t::RUNNING; timer_running.reset(); }
      else brightness *= timer_starting.ratio();
    }
    if (state == state_t::RUNNING) {
      if (timer_running.update()) { state = state_t::ENDING; timer_ending.reset(); }
    }
    if (state == state_t::ENDING) {
      if (timer_ending.update()) { state = state_t::INACTIVE; brightness = 0; return; }
      else brightness *= (1 - timer_ending.ratio());
    }

    phase += dt;
    flipTimer += dt;

    // Flip hourglass every 10 seconds
    if (flipTimer > 10.0f) {
      flipTimer = 0;
      // Flip all grains
      for (int i = 0; i < numGrains; i++) {
        grains[i].y = -grains[i].y;
        grains[i].falling = false;
      }
      flipped = !flipped;
    }

    float gravity = 8.0f;

    // Update sand physics
    for (int i = 0; i < numGrains; i++) {
      if (grains[i].y > 0) {
        // Check if near the center neck - start falling
        float r = sqrtf(grains[i].x * grains[i].x + grains[i].z * grains[i].z);
        float maxR = hourglassRadius(grains[i].y);

        if (grains[i].y < 1.5f && r < 1.0f) {
          grains[i].falling = true;
        }

        if (grains[i].falling) {
          // Fall through center and into bottom
          grains[i].y -= gravity * dt;

          // Constrain to hourglass shape
          float curMaxR = hourglassRadius(grains[i].y);
          if (r > curMaxR) {
            float scale = curMaxR / r;
            grains[i].x *= scale;
            grains[i].z *= scale;
          }
        } else {
          // Slide toward center
          if (grains[i].y > 0.5f) {
            float slideSpeed = 1.0f;
            grains[i].y -= slideSpeed * dt * 0.5f;

            // Slight inward drift
            if (r > 0.1f) {
              float drift = slideSpeed * dt * 0.3f;
              grains[i].x -= grains[i].x / r * drift;
              grains[i].z -= grains[i].z / r * drift;
            }
          }
        }
      } else {
        // In bottom half, settle
        if (grains[i].falling) {
          grains[i].y -= gravity * dt * 0.3f;
          float floorY = -6.5f;
          // Pile up: floor rises as grains accumulate
          if (grains[i].y < floorY) {
            grains[i].y = floorY;
            grains[i].falling = false;
            // Spread out slightly
            grains[i].x += noise.nextRandom(-0.5f, 0.5f);
            grains[i].z += noise.nextRandom(-0.5f, 0.5f);
          }
          // Constrain to hourglass shape
          float r2 = sqrtf(grains[i].x * grains[i].x + grains[i].z * grains[i].z);
          float curMaxR = hourglassRadius(grains[i].y);
          if (r2 > curMaxR) {
            float scale = curMaxR / r2;
            grains[i].x *= scale;
            grains[i].z *= scale;
          }
        }
      }
    }

    // Draw hourglass wireframe
    Color frameColor = Color(100, 120, 160).scaled(brightness);
    int frameSteps = 32;
    for (int i = 0; i < frameSteps; i++) {
      float a = (float)i / frameSteps * 2.0f * PI;
      // Top ring
      float topR = hourglassRadius(6.5f);
      voxel(Vector3(cosf(a) * topR, 6.5f, sinf(a) * topR), frameColor);
      // Bottom ring
      voxel(Vector3(cosf(a) * topR, -6.5f, sinf(a) * topR), frameColor);
      // Neck
      float neckR = hourglassRadius(0);
      voxel(Vector3(cosf(a) * neckR, 0, sinf(a) * neckR), frameColor);
    }
    // Vertical struts
    for (int s = 0; s < 4; s++) {
      float a = s * PI / 2.0f;
      for (int y = -13; y <= 13; y++) {
        float fy = y * 0.5f;
        float r = hourglassRadius(fy);
        voxel(Vector3(cosf(a) * r, fy, sinf(a) * r), frameColor);
      }
    }

    // Draw sand grains
    for (int i = 0; i < numGrains; i++) {
      // Sandy color with slight variation
      uint8_t sandR = 220 + (uint8_t)(noise.nextRandom(-20, 20));
      uint8_t sandG = 180 + (uint8_t)(noise.nextRandom(-15, 15));
      uint8_t sandB = 80;
      Color sandColor(sandR, sandG, sandB);
      if (grains[i].falling) {
        sandColor = Color(255, 200, 100);  // Brighter when falling
      }
      sandColor.scale(brightness);
      voxel(Vector3(grains[i].x, grains[i].y, grains[i].z), sandColor);
    }
  }
};
#endif
