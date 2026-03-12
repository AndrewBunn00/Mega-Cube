#ifndef RIPPLE_H
#define RIPPLE_H

#include "Animation.h"

class Ripple : public Animation {
 private:
  static const int MAX_SRC = 5;

  struct Source {
    float x, y, z;
    float time;
  };

  Source sources[MAX_SRC];
  float timer = 0;
  int nextSrc = 0;
  uint16_t hue16 = 0;

  static constexpr auto &settings = config.animation.ripple;

 public:
  void init() {
    state = state_t::STARTING;
    timer_starting = settings.starttime;
    timer_running = settings.runtime;
    timer_ending = settings.endtime;
    timer = 0;
    nextSrc = 0;
    hue16 = 0;

    for (int i = 0; i < MAX_SRC; i++) {
      sources[i].time = -100.0f;  // inactive
    }
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

    hue16 += (uint16_t)(dt * 1024);

    // Update all source times
    for (int i = 0; i < MAX_SRC; i++) {
      sources[i].time += dt;
    }

    // Spawn a new source every 1.5 seconds
    timer += dt;
    if (timer >= 1.5f) {
      timer -= 1.5f;
      sources[nextSrc].x = noise.nextRandom(-5.0f, 5.0f);
      sources[nextSrc].y = noise.nextRandom(-5.0f, 5.0f);
      sources[nextSrc].z = noise.nextRandom(-5.0f, 5.0f);
      sources[nextSrc].time = 0.0f;
      nextSrc = (nextSrc + 1) % MAX_SRC;
    }

    // For each voxel, compute wave contribution from all sources
    for (int x = 0; x < 16; x++) {
      for (int y = 0; y < 16; y++) {
        for (int z = 0; z < 16; z++) {
          float px = (float)x - 7.5f;
          float py = (float)y - 7.5f;
          float pz = (float)z - 7.5f;

          float val = 0;
          for (int s = 0; s < MAX_SRC; s++) {
            if (sources[s].time < 0 || sources[s].time > 6.0f) continue;

            float dx = px - sources[s].x;
            float dy = py - sources[s].y;
            float dz = pz - sources[s].z;
            float dist = sqrtf(dx * dx + dy * dy + dz * dz);

            // Expanding spherical wave with sine
            float wave = sinf(dist * 2.0f - sources[s].time * 6.0f);
            // Fade over time
            float fade = 1.0f - sources[s].time / 6.0f;
            // Fade with distance
            float distFade = 1.0f / (1.0f + dist * 0.15f);
            val += wave * fade * distFade;
          }

          // Draw above threshold
          if (fabsf(val) > 0.2f) {
            float intensity = (fabsf(val) - 0.2f) / 0.8f;
            if (intensity > 1.0f) intensity = 1.0f;
            uint8_t hue = (uint8_t)(hue16 >> 8) + (uint8_t)(val * 40.0f);
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
