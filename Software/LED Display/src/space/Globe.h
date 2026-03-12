#ifndef GLOBE_H
#define GLOBE_H

#include "Animation.h"

class Globe : public Animation {
 private:
  float angle = 0;

  static constexpr auto &settings = config.animation.globe;

 public:
  void init() {
    state = state_t::STARTING;
    timer_starting = settings.starttime;
    timer_running = settings.runtime;
    timer_ending = settings.endtime;
    angle = 0;
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

    angle += dt * 30.0f;  // degrees per second rotation
    Quaternion rot = Quaternion(angle, Vector3(0, 1, 0));

    float radius = 7.0f;

    // Draw a sphere using voxels
    for (int x = 0; x < 16; x++)
      for (int y = 0; y < 16; y++)
        for (int z = 0; z < 16; z++) {
          float fx = x - 7.5f;
          float fy = y - 7.5f;
          float fz = z - 7.5f;
          float dist = sqrtf(fx * fx + fy * fy + fz * fz);

          // Only draw voxels on the spherical shell
          if (dist < radius - 0.8f || dist > radius + 0.8f) continue;

          // Rotate the point to get the "geographical" coordinates
          Vector3 p(fx, fy, fz);
          // Inverse rotation to find what part of the globe this voxel maps to
          Quaternion invRot = Quaternion(-angle, Vector3(0, 1, 0));
          Vector3 geo = invRot.rotate(p);

          // Calculate latitude and longitude from geographical position
          float lat = asinf(geo.y / dist);  // -PI/2 to PI/2
          float lon = atan2f(geo.z, geo.x);  // -PI to PI

          // Use noise to create land/water pattern
          float noiseScale = 2.0f;
          float n = noise.noise3(geo.x * noiseScale * 0.15f + 100,
                                 geo.y * noiseScale * 0.15f + 100,
                                 geo.z * noiseScale * 0.15f + 100);

          // Polar ice caps
          bool iceCap = fabsf(lat) > 1.2f;

          Color col;
          if (iceCap) {
            col = Color(240, 245, 255);  // White ice
          } else if (n > 0.55f) {
            // Land: green to brown based on elevation (noise value)
            float elev = (n - 0.55f) / 0.45f;
            if (elev > 0.6f)
              col = Color(139, 119, 101);  // Mountain brown
            else
              col = Color(34, (uint8_t)(139 - elev * 60), 34);  // Forest green
          } else {
            // Water: deeper blue for lower noise
            float depth = (0.55f - n) / 0.55f;
            col = Color(0, (uint8_t)(80 + (1.0f - depth) * 80), (uint8_t)(140 + (1.0f - depth) * 80));
          }

          col.scale(brightness);
          voxel(x, y, z, col);
        }
  }
};
#endif
