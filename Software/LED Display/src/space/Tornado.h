#ifndef TORNADO_H
#define TORNADO_H

#include "Animation.h"

class Tornado : public Animation {
 private:
  static const int NUM_PARTICLES = 300;

  struct TParticle {
    float angle;
    float y;
    float radius;
    float speed;
    uint8_t hue;
  };

  TParticle parts[NUM_PARTICLES];
  float phase = 0;

  static constexpr auto &settings = config.animation.tornado;

 public:
  void init() {
    state = state_t::STARTING;
    timer_starting = settings.starttime;
    timer_running = settings.runtime;
    timer_ending = settings.endtime;
    phase = 0;

    for (int i = 0; i < NUM_PARTICLES; i++) {
      parts[i].angle = noise.nextRandom(0.0f, 2.0f * PI);
      parts[i].y = noise.nextRandom(-7.5f, 7.5f);
      parts[i].radius = noise.nextRandom(0.3f, 1.0f);
      parts[i].speed = noise.nextRandom(1.5f, 4.0f);
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

    for (int i = 0; i < NUM_PARTICLES; i++) {
      // Move particles upward and rotate
      parts[i].angle += dt * parts[i].speed * 2.0f;
      parts[i].y += dt * parts[i].speed * 0.8f;

      // Wrap around when past top
      if (parts[i].y > 7.5f) {
        parts[i].y -= 15.0f;
        parts[i].hue = (uint8_t)(int)noise.nextRandom(0, 256);
      }

      // Normalized height 0..1 from bottom to top
      float t = (parts[i].y + 7.5f) / 15.0f;

      // Radius widens with height: narrow at bottom, wide at top
      float effectiveRadius = parts[i].radius * (1.0f + t * 5.0f);

      float px = cosf(parts[i].angle) * effectiveRadius;
      float pz = sinf(parts[i].angle) * effectiveRadius;
      float py = parts[i].y;

      Vector3 pos = Vector3(px, py, pz);

      // Color shifts with height
      uint8_t hue = parts[i].hue + (uint8_t)(t * 40.0f);
      Color c = Color(hue, RainbowGradientPalette);
      c.scale(brightness);
      voxel(pos, c);
    }
  }
};
#endif
