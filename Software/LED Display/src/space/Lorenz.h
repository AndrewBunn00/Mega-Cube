#ifndef LORENZ_H
#define LORENZ_H

#include "Animation.h"

class Lorenz : public Animation {
 private:
  static const int TRAIL_LEN = 500;
  Vector3 trail[TRAIL_LEN];
  int trailHead = 0;
  int trailCount = 0;
  float lx, ly, lz;
  float angle = 0;
  float sigma = 10.0f, rho = 28.0f, beta = 8.0f / 3.0f;

  static constexpr auto &settings = config.animation.lorenz;

 public:
  void init() {
    state = state_t::STARTING;
    timer_starting = settings.starttime;
    timer_running = settings.runtime;
    timer_ending = settings.endtime;
    lx = 1; ly = 1; lz = 1;
    trailHead = 0; trailCount = 0;
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

    angle += dt * 0.2f;
    Quaternion q = Quaternion(angle * 30, Vector3(0, 1, 0));

    float step = 0.005f;
    int steps = (int)(dt / step) + 1;
    for (int s = 0; s < steps; s++) {
      float dx = sigma * (ly - lx) * step;
      float dy = (lx * (rho - lz) - ly) * step;
      float dz = (lx * ly - beta * lz) * step;
      lx += dx; ly += dy; lz += dz;

      Vector3 p(lx / 20.0f * 6, (lz - 25) / 25.0f * 6, ly / 20.0f * 6);
      trail[trailHead] = p;
      trailHead = (trailHead + 1) % TRAIL_LEN;
      if (trailCount < TRAIL_LEN) trailCount++;
    }

    for (int i = 0; i < trailCount; i++) {
      int idx = (trailHead - trailCount + i + TRAIL_LEN) % TRAIL_LEN;
      float t = (float)i / trailCount;
      Color c = Color((uint8_t)(t * 255), RainbowGradientPalette);
      c.scale((uint8_t)(t * 255));
      voxel(q.rotate(trail[idx]), c);
    }
  }
};
#endif
