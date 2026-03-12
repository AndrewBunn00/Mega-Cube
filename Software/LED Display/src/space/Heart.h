#ifndef HEART_H
#define HEART_H

#include "Animation.h"

class Heart : public Animation {
 private:
  float phase = 0;

  static constexpr auto &settings = config.animation.heart;

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

    phase += dt * 3.0f;

    // Pulsing scale for heartbeat effect
    float beat = 1.0f + 0.15f * sinf(phase) * sinf(phase);
    // Slow rotation
    Quaternion rot = Quaternion(phase * 12.0f, Vector3(0, 1, 0));

    for (int ix = 0; ix < 16; ix++)
      for (int iy = 0; iy < 16; iy++)
        for (int iz = 0; iz < 16; iz++) {
          // Map to -1..1 range
          float px = (ix - 7.5f) / (7.5f * beat);
          float py = (iy - 7.5f) / (7.5f * beat);
          float pz = (iz - 7.5f) / (7.5f * beat);

          // Apply inverse rotation to test in heart's local space
          Quaternion invRot = Quaternion(-phase * 12.0f, Vector3(0, 1, 0));
          Vector3 lp = invRot.rotate(Vector3(px, py, pz));
          float x = lp.x;
          float y = lp.y;
          float z = lp.z;

          // Heart implicit surface equation:
          // (x^2 + (9/4)*y^2 + z^2 - 1)^3 - x^2*z^3 - (9/80)*y^2*z^3 <= 0
          // Flip y so heart points up
          float yh = -y + 0.1f;

          float x2 = x * x;
          float y2 = yh * yh;
          float z2 = z * z;

          float a = x2 + 2.25f * y2 + z2 - 1.0f;
          float val = a * a * a - x2 * z2 * z - 0.1125f * y2 * z2 * z;

          if (val <= 0.0f) {
            // Depth-based red coloring: deeper = darker red
            float depth = sqrtf(x2 + y2 + z2);
            uint8_t r = (uint8_t)(255 - depth * 60);
            if (r < 80) r = 80;
            uint8_t g = (uint8_t)(20 - depth * 15);
            if (g > 250) g = 0;  // unsigned underflow protection
            uint8_t b = (uint8_t)(40 - depth * 20);
            if (b > 250) b = 0;

            Color col(r, g, b);
            col.scale(brightness);
            voxel(ix, iy, iz, col);
          }
        }
  }
};
#endif
