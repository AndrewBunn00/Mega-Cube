#ifndef FIREFLIES_H
#define FIREFLIES_H

#include "Animation.h"

class Fireflies : public Animation {
 private:
  struct Firefly { float x, y, z, vx, vy, vz, phase, speed; uint8_t hue; };
  static const int NUM_FLIES = 40;
  Firefly flies[NUM_FLIES];

  static constexpr auto &settings = config.animation.fireflies;

 public:
  void init() {
    state = state_t::STARTING;
    timer_starting = settings.starttime;
    timer_running = settings.runtime;
    timer_ending = settings.endtime;
    for (int i = 0; i < NUM_FLIES; i++) {
      flies[i].x = noise.nextRandom(-7, 7);
      flies[i].y = noise.nextRandom(-7, 7);
      flies[i].z = noise.nextRandom(-7, 7);
      flies[i].vx = noise.nextRandom(-1, 1);
      flies[i].vy = noise.nextRandom(-1, 1);
      flies[i].vz = noise.nextRandom(-1, 1);
      flies[i].phase = noise.nextRandom(0, 2.0f * PI);
      flies[i].speed = noise.nextRandom(0.5f, 2.0f);
      flies[i].hue = (uint8_t)noise.nextRandom(0, 256);
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

    for (int i = 0; i < NUM_FLIES; i++) {
      Firefly& f = flies[i];
      f.phase += dt * f.speed;
      float glow = fmaxf(0.0f, sinf(f.phase));
      glow *= glow;

      f.x += f.vx * dt; f.y += f.vy * dt; f.z += f.vz * dt;
      f.vx += noise.nextRandom(-2, 2) * dt;
      f.vy += noise.nextRandom(-2, 2) * dt;
      f.vz += noise.nextRandom(-2, 2) * dt;
      float speed = sqrtf(f.vx*f.vx + f.vy*f.vy + f.vz*f.vz);
      if (speed > 2.0f) { f.vx *= 2.0f/speed; f.vy *= 2.0f/speed; f.vz *= 2.0f/speed; }

      if (f.x < -7 || f.x > 7) f.vx = -f.vx;
      if (f.y < -7 || f.y > 7) f.vy = -f.vy;
      if (f.z < -7 || f.z > 7) f.vz = -f.vz;
      f.x = fmaxf(-7.5f, fminf(7.5f, f.x));
      f.y = fmaxf(-7.5f, fminf(7.5f, f.y));
      f.z = fmaxf(-7.5f, fminf(7.5f, f.z));

      if (glow > 0.05f) {
        Color c = Color(f.hue, RainbowGradientPalette);
        c.scale((uint8_t)(glow * 255));
        radiate5(Vector3(f.x, f.y, f.z), c, 2.0f);
      }
    }
  }
};
#endif
