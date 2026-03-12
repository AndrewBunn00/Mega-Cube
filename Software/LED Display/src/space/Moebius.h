#ifndef MOEBIUS_H
#define MOEBIUS_H

#include "Animation.h"

class Moebius : public Animation {
 private:
  float angle = 0;
  uint16_t hue16 = 0;

  static constexpr auto &settings = config.animation.moebius;

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
    Quaternion q = Quaternion(angle * 25, Vector3(0, 1, 0));
    float R = 5.0f, width = 2.5f;
    int resT = 60, resS = 8;

    for (int i = 0; i < resT; i++) {
      float t = (float)i / resT * (2.0f * PI);
      for (int j = 0; j < resS; j++) {
        float s = ((float)j / (resS - 1) - 0.5f) * width;
        float x = (R + s * cosf(t / 2)) * cosf(t);
        float y = s * sinf(t / 2);
        float z = (R + s * cosf(t / 2)) * sinf(t);
        Color c = Color((uint8_t)((hue16 >> 8) + i * 3), RainbowGradientPalette);
        radiate(q.rotate(Vector3(x, y, z)), c, 1.0f);
      }
    }
  }
};
#endif
