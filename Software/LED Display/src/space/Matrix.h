#ifndef MATRIX_H
#define MATRIX_H

#include "Animation.h"

class Matrix : public Animation {
 private:
  struct Column {
    float y;
    float speed;
    uint8_t length;
  };

  Column cols[16][16];

  static constexpr auto &settings = config.animation.matrix;

  void resetColumn(int x, int z) {
    cols[x][z].y = noise.nextRandom(16.0f, 32.0f);
    cols[x][z].speed = noise.nextRandom(4.0f, 12.0f);
    cols[x][z].length = 4 + (int)noise.nextRandom(0, 8);
  }

 public:
  void init() {
    state = state_t::STARTING;
    timer_starting = settings.starttime;
    timer_running = settings.runtime;
    timer_ending = settings.endtime;

    for (int x = 0; x < 16; x++) {
      for (int z = 0; z < 16; z++) {
        cols[x][z].y = noise.nextRandom(0.0f, 32.0f);
        cols[x][z].speed = noise.nextRandom(4.0f, 12.0f);
        cols[x][z].length = 4 + (int)noise.nextRandom(0, 8);
      }
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

    for (int x = 0; x < 16; x++) {
      for (int z = 0; z < 16; z++) {
        // Move column downward
        cols[x][z].y -= cols[x][z].speed * dt;

        float headY = cols[x][z].y;
        uint8_t length = cols[x][z].length;

        // Reset column when it has fully passed below the cube
        if (headY < -(float)length) {
          resetColumn(x, z);
          continue;
        }

        // Draw the column trail
        for (int t = 0; t < length; t++) {
          int py = (int)(headY + t + 0.5f);
          if (py < 0 || py > 15) continue;

          // Map from cube coords to system coords
          float sx = (float)x - 7.5f;
          float sy = (float)py - 7.5f;
          float sz = (float)z - 7.5f;

          if (t == 0) {
            // White tip (head of the column)
            Color c = Color::WHITE;
            c.scale(brightness);
            voxel(Vector3(sx, sy, sz), c);
          } else {
            // Green trail that fades
            float fade = 1.0f - (float)t / (float)length;
            uint8_t green = (uint8_t)(255.0f * fade * fade);
            Color c = Color(0, green, 0);
            c.scale(brightness);
            voxel(Vector3(sx, sy, sz), c);
          }
        }
      }
    }
  }
};
#endif
