#ifndef INTERFERENCE_H
#define INTERFERENCE_H

#include "Animation.h"

class Interference : public Animation {
 private:
  float phase = 0;
  uint16_t hue16 = 0;

  static constexpr auto &settings = config.animation.interference;

 public:
  void init() {
    state = state_t::STARTING;
    timer_starting = settings.starttime;
    timer_running = settings.runtime;
    timer_ending = settings.endtime;
    phase = 0;
    hue16 = 0;
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
    hue16 += (uint16_t)(dt * 512);

    // 4 wave sources at cube corners
    float sx[4] = {-7.0f, 7.0f, -7.0f, 7.0f};
    float sy[4] = {-7.0f, -7.0f, 7.0f, 7.0f};
    float sz[4] = {-7.0f, 7.0f, 7.0f, -7.0f};
    float freq[4] = {3.0f, 3.5f, 4.0f, 2.5f};

    for (int x = 0; x < 16; x++) {
      for (int y = 0; y < 16; y++) {
        for (int z = 0; z < 16; z++) {
          float px = (float)x - 7.5f;
          float py = (float)y - 7.5f;
          float pz = (float)z - 7.5f;

          float val = 0;
          for (int s = 0; s < 4; s++) {
            float dx = px - sx[s];
            float dy = py - sy[s];
            float dz = pz - sz[s];
            float dist = sqrtf(dx * dx + dy * dy + dz * dz);
            val += sinf(dist * freq[s] * 0.5f - phase * 4.0f);
          }
          val /= 4.0f;

          // Draw if absolute value exceeds threshold
          if (fabsf(val) > 0.3f) {
            float intensity = (fabsf(val) - 0.3f) / 0.7f;
            if (intensity > 1.0f) intensity = 1.0f;
            // Rainbow palette coloring
            uint8_t hue = (uint8_t)(hue16 >> 8) + (uint8_t)((val + 1.0f) * 64.0f);
            Color c = Color(hue, RainbowGradientPalette);
            c.scale((uint8_t)(intensity * brightness));
            voxel(x, y, z, c);
          }
        }
      }
    }
  }
};
#endif
