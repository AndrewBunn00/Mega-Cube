#ifndef EXPLOSION_H
#define EXPLOSION_H

#include "Animation.h"

class Explosion : public Animation {
 private:
  static const int NUM_P = 250;

  struct EPart {
    Vector3 dir;
    float dist;
    uint8_t hue;
  };

  EPart parts[NUM_P];
  float phase = 0;

  static constexpr auto &settings = config.animation.explosion;

 public:
  void init() {
    state = state_t::STARTING;
    timer_starting = settings.starttime;
    timer_running = settings.runtime;
    timer_ending = settings.endtime;
    phase = 0;

    for (int i = 0; i < NUM_P; i++) {
      // Random direction on unit sphere, then normalize
      parts[i].dir = Vector3(
          noise.nextRandom(-1.0f, 1.0f),
          noise.nextRandom(-1.0f, 1.0f),
          noise.nextRandom(-1.0f, 1.0f)).normalized();
      parts[i].dist = noise.nextRandom(0.5f, 1.0f);
      parts[i].hue = (uint8_t)(int)noise.nextRandom(0, 256);
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

    phase += dt;

    // Looping expand/contract cycle with 3 second period
    float cycle = fmodf(phase, 3.0f) / 3.0f;
    // Smooth expansion: use sine for organic motion
    float expansion;
    if (cycle < 0.5f) {
      // Expanding phase (0 to 1)
      expansion = sinf(cycle * PI);
    } else {
      // Contracting phase (1 to 0)
      expansion = sinf(cycle * PI);
    }

    for (int i = 0; i < NUM_P; i++) {
      float radius = parts[i].dist * expansion * 7.5f;
      Vector3 pos = parts[i].dir * radius;

      // LavaPalette colors based on expansion and particle hue
      uint8_t palIdx = parts[i].hue + (uint8_t)(expansion * 64.0f);
      Color c = Color(palIdx, LavaPalette);
      c.scale(brightness);
      radiate(pos, c, 1.5f);
    }
  }
};
#endif
