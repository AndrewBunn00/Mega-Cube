#ifndef OCEAN_H
#define OCEAN_H

#include "Animation.h"

class Ocean : public Animation {
 private:
  float phase = 0;

  static constexpr auto &settings = config.animation.ocean;

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

    phase += dt;
    for (int x = 0; x < 16; x++) {
      for (int z = 0; z < 16; z++) {
        float fx = (x - 7.5f) / 7.5f;
        float fz = (z - 7.5f) / 7.5f;
        float h = sinf(fx * 3 + phase * 2) * 0.3f
                + sinf(fz * 4 + phase * 1.5f) * 0.25f
                + sinf((fx + fz) * 2.5f + phase * 1.8f) * 0.2f;
        int surfaceY = (int)(7.5f + h * 5);
        if (surfaceY > 15) surfaceY = 15;

        for (int y = 0; y <= surfaceY && y < 16; y++) {
          float depthRatio = (float)(surfaceY - y) / (float)(surfaceY + 1);
          float n = noise.noise3(x * 0.25f, y * 0.3f, z * 0.25f + phase * 0.3f) * 0.3f;

          if (y >= surfaceY - 1) {
            // Foam/crest - bright white-blue
            float foam = 0.7f + 0.3f * sinf(fx * 5 + fz * 3 + phase * 3);
            voxel((uint8_t)x, (uint8_t)y, (uint8_t)z,
                  Color((uint8_t)(180*foam), (uint8_t)(220*foam), 255));
          } else if (depthRatio < 0.3f) {
            // Shallow - light blue-green
            float t = depthRatio / 0.3f;
            uint8_t r = (uint8_t)((30 - 20*t + n*30));
            uint8_t g = (uint8_t)((160 - 40*t + n*40));
            uint8_t b = (uint8_t)((220 - 20*t + n*20));
            voxel((uint8_t)x, (uint8_t)y, (uint8_t)z, Color(r, g, b));
          } else if (depthRatio < 0.6f) {
            // Mid depth - medium blue
            float t = (depthRatio - 0.3f) / 0.3f;
            uint8_t r = (uint8_t)(10 - 5*t);
            uint8_t g = (uint8_t)((120 - 50*t + n*30));
            uint8_t b = (uint8_t)((200 - 30*t + n*20));
            voxel((uint8_t)x, (uint8_t)y, (uint8_t)z, Color(r, g, b));
          } else {
            // Deep - dark navy/black
            float t = (depthRatio - 0.6f) / 0.4f;
            uint8_t r = (uint8_t)(5 * (1-t));
            uint8_t g = (uint8_t)((70 - 50*t + n*20));
            uint8_t b = (uint8_t)((170 - 100*t + n*20));
            voxel((uint8_t)x, (uint8_t)y, (uint8_t)z, Color(r, g, b));
          }
        }
      }
    }
  }
};
#endif
