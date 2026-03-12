#ifndef TORUS_H
#define TORUS_H

#include "Animation.h"

class Torus : public Animation {
 private:
  float angle = 0;
  uint16_t hue16 = 0;

  static constexpr auto &settings = config.animation.torus;

 public:
  void init() {
    state = state_t::STARTING;
    timer_starting = settings.starttime;
    timer_running = settings.runtime;
    timer_ending = settings.endtime;
    angle = 0;
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

    angle += dt;
    hue16 += (uint16_t)(dt * 40 * 255);
    float R = 4.5f, r = 2.0f;
    Quaternion q = Quaternion(angle * 30, Vector3(1, 0, 0))
                 * Quaternion(angle * 20, Vector3(0, 0, 1));

    int resU = 40, resV = 20;
    for (int i = 0; i < resU; i++) {
      float u = (float)i / resU * (2.0f * PI);
      for (int j = 0; j < resV; j++) {
        float v = (float)j / resV * (2.0f * PI);
        Vector3 p((R + r * cosf(v)) * cosf(u),
                  (R + r * cosf(v)) * sinf(u),
                  r * sinf(v));
        Color c = Color((uint8_t)((hue16 >> 8) + i * 5), RainbowGradientPalette);
        radiate(q.rotate(p), c, 1.2f);
      }
    }
  }
};
#endif
