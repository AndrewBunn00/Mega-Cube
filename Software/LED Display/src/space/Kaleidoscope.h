#ifndef KALEIDOSCOPE_H
#define KALEIDOSCOPE_H

#include "Animation.h"

class Kaleidoscope : public Animation {
 private:
  float phase = 0;

  static constexpr auto &settings = config.animation.kaleidoscope;

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

    phase += dt * 0.3f;

    // Compute noise in one octant (x: 0-7, y: 0-7, z: 0-7) and mirror across
    // all 3 axes to create 8-way symmetric kaleidoscope effect
    float scale = 0.25f;

    for (int x = 0; x <= 7; x++) {
      for (int y = 0; y <= 7; y++) {
        for (int z = 0; z <= 7; z++) {
          // Sample 4D noise for animation
          float nx = x * scale;
          float ny = y * scale;
          float nz = z * scale;
          float nw = phase;

          float val = noise.noise4(nx, ny, nz, nw);

          // Threshold: only draw above 0.35
          if (val < 0.35f) continue;

          // Map to rainbow palette
          uint8_t palIdx = (uint8_t)(val * 255.0f);
          Color c = Color(palIdx, RainbowGradientPalette);
          float intensity = (val - 0.35f) / 0.65f;
          c.scale((uint8_t)(intensity * brightness));

          // Mirror across all 3 axes (8 copies)
          // Octant coords in system space: x maps to 0..7 -> 0.5..7.5 in cube
          // We use direct cube coordinates for mirroring
          int mx = 15 - x;  // mirrored x
          int my = 15 - y;  // mirrored y
          int mz = 15 - z;  // mirrored z

          // 8 symmetric positions
          voxel(x, y, z, c);
          voxel(mx, y, z, c);
          voxel(x, my, z, c);
          voxel(x, y, mz, c);
          voxel(mx, my, z, c);
          voxel(mx, y, mz, c);
          voxel(x, my, mz, c);
          voxel(mx, my, mz, c);
        }
      }
    }
  }
};
#endif
