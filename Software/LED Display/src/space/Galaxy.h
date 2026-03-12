#ifndef GALAXY_H
#define GALAXY_H

#include "Animation.h"

class Galaxy : public Animation {
 private:
  static const int NUM_STARS = 600;

  struct Star {
    float r;
    float armAngle;
    float spread_x;
    float spread_z;
    float height;
    uint8_t arm;
  };

  Star stars[NUM_STARS];
  float angle = 0;

  static constexpr auto &settings = config.animation.galaxy;

 public:
  void init() {
    state = state_t::STARTING;
    timer_starting = settings.starttime;
    timer_running = settings.runtime;
    timer_ending = settings.endtime;
    angle = 0;

    for (int i = 0; i < NUM_STARS; i++) {
      stars[i].r = noise.nextRandom(0.0f, 1.0f);
      stars[i].armAngle = noise.nextRandom(0.0f, 2.0f * PI);
      stars[i].spread_x = noise.nextGaussian(0.0f, 0.15f);
      stars[i].spread_z = noise.nextGaussian(0.0f, 0.15f);
      stars[i].height = noise.nextGaussian(0.0f, 0.08f);
      stars[i].arm = (uint8_t)(int)noise.nextRandom(0, 3);
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

    angle += dt * 0.15f;

    // Tilt the galaxy 30 degrees and rotate it
    Quaternion tilt = Quaternion(30.0f, Vector3(1, 0, 0));
    Quaternion spin = Quaternion(angle * 60.0f, Vector3(0, 1, 0));
    Quaternion q = spin * tilt;

    // Bright galactic core
    Color coreColor = Color(255, 220, 180);
    radiate5(q.rotate(Vector3(0, 0, 0)), coreColor.scale(brightness), 3.0f);

    // Render each star along spiral arms
    for (int i = 0; i < NUM_STARS; i++) {
      float r = stars[i].r;
      // Spiral arm angle: base arm offset + winding with radius + individual spread
      float armOffset = stars[i].arm * (2.0f * PI / 3.0f);
      float spiralAngle = armOffset + stars[i].armAngle + r * 4.0f + angle * 2.0f;

      float px = r * cosf(spiralAngle) + stars[i].spread_x * r;
      float pz = r * sinf(spiralAngle) + stars[i].spread_z * r;
      float py = stars[i].height * (1.0f - r);

      Vector3 pos = q.rotate(Vector3(px, py, pz) * 7.5f);

      // Color gradient: warm center (yellow/orange) to blue outer
      uint8_t hueVal;
      if (r < 0.3f) {
        // Warm center: orange to yellow (hue ~32-48)
        hueVal = 32 + (uint8_t)(r * 50.0f);
      } else if (r < 0.6f) {
        // Mid region: white-blue (hue ~128-160)
        hueVal = 128 + (uint8_t)((r - 0.3f) * 100.0f);
      } else {
        // Outer: deep blue (hue ~160-200)
        hueVal = 160 + (uint8_t)((r - 0.6f) * 100.0f);
      }
      Color c = Color(hueVal, RainbowGradientPalette);
      float starBright = (1.0f - r * 0.6f);
      c.scale((uint8_t)(starBright * brightness));
      voxel(pos, c);
    }
  }
};
#endif
