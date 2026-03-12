#ifndef AURORA_H
#define AURORA_H

#include "Animation.h"

class Aurora : public Animation {
 private:
  float phase = 0;

  static constexpr auto &settings = config.animation.aurora;

 public:
  void init() {
    state = state_t::STARTING;
    timer_starting = settings.starttime;
    timer_running = settings.runtime;
    timer_ending = settings.endtime;
    phase = 0;
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

    phase += dt * 0.3f;
    for (int x = 0; x < 16; x++) {
      for (int z = 0; z < 16; z++) {
        float h1 = 10 + 4 * noise.noise3(x * 0.15f + phase, z * 0.15f, phase * 0.5f);
        float h2 = 8 + 3 * noise.noise3(x * 0.2f, z * 0.2f + phase * 0.7f, phase * 0.3f);
        for (int y = 0; y < 16; y++) {
          float b1 = fmaxf(0.0f, 1.0f - fabsf(y - h1) * 0.4f);
          float b2 = fmaxf(0.0f, 0.7f - fabsf(y - h2) * 0.35f);
          float br = b1 + b2;
          if (br > 0.05f) {
            br = fminf(br, 1.0f);
            float t = y / 15.0f;
            uint8_t r = (uint8_t)(t * 150 * br);
            uint8_t g = (uint8_t)((1.0f - t * 0.5f) * 255 * br);
            uint8_t b = (uint8_t)(t * 200 * br);
            voxel((uint8_t)x, (uint8_t)y, (uint8_t)z, Color(r, g, b));
          }
        }
      }
    }
  }
};
#endif
