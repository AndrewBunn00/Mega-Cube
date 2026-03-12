#ifndef SIERPINSKI_H
#define SIERPINSKI_H

#include "Animation.h"

class Sierpinski : public Animation {
 private:
  static const int NUM_POINTS = 2000;
  Vector3 points[NUM_POINTS];
  float angle = 0;
  uint16_t hue16 = 0;
  Vector3 vertices[4] = {
    Vector3(1, 1, 1), Vector3(1, -1, -1),
    Vector3(-1, 1, -1), Vector3(-1, -1, 1)
  };

  static constexpr auto &settings = config.animation.sierpinski;

 public:
  void init() {
    state = state_t::STARTING;
    timer_starting = settings.starttime;
    timer_running = settings.runtime;
    timer_ending = settings.endtime;
    angle = 0;
    hue16 = 0;
    Vector3 p(0, 0, 0);
    for (int i = 0; i < NUM_POINTS; i++) {
      p = (p + vertices[(int)noise.nextRandom(0, 4)] * 6.0f) * 0.5f;
      points[i] = p;
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

    angle += dt * 0.5f;
    hue16 += (uint16_t)(dt * 30 * 255);
    Quaternion q = Quaternion(angle * 30, Vector3(0, 1, 0));
    for (int i = 0; i < NUM_POINTS; i++) {
      Color c = Color((uint8_t)((hue16 >> 8) + (uint8_t)(points[i].y * 15)), RainbowGradientPalette);
      voxel(q.rotate(points[i]), c);
    }
  }
};
#endif
