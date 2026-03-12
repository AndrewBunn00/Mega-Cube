#ifndef SPIROGRAPH_H
#define SPIROGRAPH_H

#include "Animation.h"

class Spirograph : public Animation {
 private:
  float phase = 0;
  static const int TRAIL_LEN = 600;
  Vector3 trail[TRAIL_LEN];
  int trailCount = 0;
  int trailHead = 0;

  static constexpr auto &settings = config.animation.spirograph;

 public:
  void init() {
    state = state_t::STARTING;
    timer_starting = settings.starttime;
    timer_running = settings.runtime;
    timer_ending = settings.endtime;
    phase = 0;
    trailCount = 0;
    trailHead = 0;
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
    float t = phase * 2;
    float a = 3 + sinf(phase * 0.1f);
    float b = 2 + cosf(phase * 0.13f);
    float cp = 5 + sinf(phase * 0.07f);
    float d = 3 + cosf(phase * 0.11f);
    float x = sinf(a * t) * cosf(b * t) * 6.5f;
    float y = sinf(cp * t) * 6.5f;
    float z = cosf(d * t) * sinf(b * t) * 6.5f;

    trail[trailHead] = Vector3(x, y, z);
    trailHead = (trailHead + 1) % TRAIL_LEN;
    if (trailCount < TRAIL_LEN) trailCount++;

    for (int i = 0; i < trailCount; i++) {
      int idx = (trailHead - trailCount + i + TRAIL_LEN) % TRAIL_LEN;
      float frac = (float)i / trailCount;
      Color col = Color((uint8_t)(frac * 255), RainbowGradientPalette);
      col.scale((uint8_t)(frac * brightness));
      voxel(trail[idx], col);
    }
  }
};
#endif
