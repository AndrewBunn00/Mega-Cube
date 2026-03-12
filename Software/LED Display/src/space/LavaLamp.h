#ifndef LAVALAMP_H
#define LAVALAMP_H

#include "Animation.h"

class LavaLamp : public Animation {
 private:
  struct Blob { float phaseY, phaseX, phaseZ, speedY, speedX, speedZ, size; };
  static const int NUM_BLOBS = 5;
  Blob blobs[NUM_BLOBS];
  float phase = 0;

  static constexpr auto &settings = config.animation.lavalamp;

 public:
  void init() {
    state = state_t::STARTING;
    timer_starting = settings.starttime;
    timer_running = settings.runtime;
    timer_ending = settings.endtime;
    phase = 0;
    for (int i = 0; i < NUM_BLOBS; i++) {
      blobs[i].phaseY = noise.nextRandom(0, 2.0f * PI);
      blobs[i].phaseX = noise.nextRandom(0, 2.0f * PI);
      blobs[i].phaseZ = noise.nextRandom(0, 2.0f * PI);
      blobs[i].speedY = noise.nextRandom(0.15f, 0.35f);
      blobs[i].speedX = noise.nextRandom(0.08f, 0.2f);
      blobs[i].speedZ = noise.nextRandom(0.08f, 0.2f);
      blobs[i].size = noise.nextRandom(2.2f, 3.2f);
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

    // Blob centers - drift through full cube space
    float bx[NUM_BLOBS], by[NUM_BLOBS], bz[NUM_BLOBS];
    for (int i = 0; i < NUM_BLOBS; i++) {
      by[i] = sinf(phase * blobs[i].speedY + blobs[i].phaseY) * 6.5f;
      bx[i] = sinf(phase * blobs[i].speedX + blobs[i].phaseX) * 4.0f;
      bz[i] = sinf(phase * blobs[i].speedZ + blobs[i].phaseZ) * 4.0f;
    }

    float threshold = 1.0f;
    for (int vx = 0; vx < 16; vx++) {
      for (int vy = 0; vy < 16; vy++) {
        for (int vz = 0; vz < 16; vz++) {
          float fx = vx - 7.5f, fy = vy - 7.5f, fz = vz - 7.5f;

          // Metaball field from blobs - they merge when close
          float field = 0;
          for (int i = 0; i < NUM_BLOBS; i++) {
            float dx = fx - bx[i], dy = fy - by[i], dz = fz - bz[i];
            float r2 = blobs[i].size * blobs[i].size;
            field += r2 / (dx*dx + dy*dy + dz*dz + 0.5f);
          }

          // Thin wax pool settled at the bottom (1-2 voxels thick)
          if (fy < -5.5f) {
            float poolDy = fy + 7.5f; // distance from very bottom
            float poolField = 1.5f / (poolDy * poolDy + 0.5f);
            field += poolField;
          }

          if (field > threshold) {
            // Depth into blob: 0 at surface, slow ramp to 1 deep inside
            float depth = fminf((field - threshold) * 0.6f, 1.0f);
            // Surface = deep red, core = bright yellow-white
            uint8_t r = (uint8_t)(100 + 155 * depth);
            uint8_t g = (uint8_t)(20 + 180 * depth * depth);
            uint8_t b = (uint8_t)(5 + 60 * depth * depth * depth);
            voxel((uint8_t)vx, (uint8_t)vy, (uint8_t)vz, Color(r, g, b));
          } else {
            // Dim liquid fill across entire cube
            float glow = 0.04f + 0.02f * sinf(fy * 0.3f + phase * 0.1f);
            glow += field * 0.04f;
            uint8_t r = (uint8_t)(100 * glow);
            uint8_t g = (uint8_t)(25 * glow);
            uint8_t b = (uint8_t)(8 * glow);
            if (r > 2)
              voxel((uint8_t)vx, (uint8_t)vy, (uint8_t)vz, Color(r, g, b));
          }
        }
      }
    }
  }
};
#endif
