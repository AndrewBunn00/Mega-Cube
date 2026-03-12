#ifndef DNA_H
#define DNA_H

#include "Animation.h"

class DNA : public Animation {
 private:
  float phase = 0;

  static constexpr auto &settings = config.animation.dna;

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

    phase += dt * 1.5f;

    float helixRadius = 4.0f;
    int steps = 80;

    Quaternion rot = Quaternion(phase * 40.0f, Vector3(0, 1, 0));

    for (int i = 0; i < steps; i++) {
      float t = (float)i / steps;
      float y = t * 15.0f - 7.5f;
      float angle1 = t * 4.0f * PI + phase;
      float angle2 = angle1 + PI;  // Second strand offset by 180 degrees

      // Strand 1 position
      float x1 = cosf(angle1) * helixRadius;
      float z1 = sinf(angle1) * helixRadius;
      Vector3 p1 = rot.rotate(Vector3(x1, y, z1));

      // Strand 2 position
      float x2 = cosf(angle2) * helixRadius;
      float z2 = sinf(angle2) * helixRadius;
      Vector3 p2 = rot.rotate(Vector3(x2, y, z2));

      // Draw strand 1 (cyan)
      Color c1 = Color(0, 200, 255);
      c1.scale(brightness);
      radiate(p1, c1, 1.2f);

      // Draw strand 2 (magenta)
      Color c2 = Color(255, 0, 200);
      c2.scale(brightness);
      radiate(p2, c2, 1.2f);

      // Draw connecting rungs every 4th step
      if (i % 4 == 0) {
        int rungSteps = 6;
        for (int r = 0; r <= rungSteps; r++) {
          float frac = (float)r / rungSteps;
          float rx = x1 + (x2 - x1) * frac;
          float rz = z1 + (z2 - z1) * frac;
          Vector3 rp = rot.rotate(Vector3(rx, y, rz));

          // Alternate rung colors: base pair colors
          Color rc;
          if (i % 8 == 0) {
            // Adenine-Thymine pair: red-green
            rc = Color((uint8_t)(255 * (1.0f - frac)), (uint8_t)(255 * frac), 0);
          } else {
            // Guanine-Cytosine pair: yellow-blue
            rc = Color((uint8_t)(255 * (1.0f - frac)), (uint8_t)(255 * (1.0f - frac)), (uint8_t)(255 * frac));
          }
          rc.scale(brightness);
          voxel(rp, rc);
        }
      }
    }
  }
};
#endif
