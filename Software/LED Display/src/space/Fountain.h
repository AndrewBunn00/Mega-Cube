#ifndef FOUNTAIN_H
#define FOUNTAIN_H

#include "Animation.h"

class Fountain : public Animation {
 private:
  static const int MAX_P = 300;

  struct FPart {
    float x, y, z;
    float vx, vy, vz;
    float life;
    uint8_t hue;
  };

  FPart parts[MAX_P];
  int numParts = 0;
  float spawnTimer = 0;
  uint16_t hue16 = 0;

  static constexpr auto &settings = config.animation.fountain;

 public:
  void init() {
    state = state_t::STARTING;
    timer_starting = settings.starttime;
    timer_running = settings.runtime;
    timer_ending = settings.endtime;
    numParts = 0;
    spawnTimer = 0;
    hue16 = 0;
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

    hue16 += (uint16_t)(dt * 512);

    // Spawn new particles from bottom center
    spawnTimer += dt;
    float spawnInterval = 0.005f;
    while (spawnTimer >= spawnInterval && numParts < MAX_P) {
      spawnTimer -= spawnInterval;
      FPart &p = parts[numParts];
      p.x = 0.0f;
      p.y = -7.5f;
      p.z = 0.0f;
      p.vy = noise.nextRandom(13.0f, 17.0f);
      float spread = noise.nextRandom(1.0f, 4.0f);
      float angle = noise.nextRandom(0.0f, 2.0f * PI);
      p.vx = cosf(angle) * spread;
      p.vz = sinf(angle) * spread;
      p.life = 1.0f;
      p.hue = (uint8_t)(hue16 >> 8);
      numParts++;
    }

    // Update and draw particles
    int alive = 0;
    for (int i = 0; i < numParts; i++) {
      // Apply gravity
      parts[i].vy -= 10.0f * dt;
      // Move
      parts[i].x += parts[i].vx * dt;
      parts[i].y += parts[i].vy * dt;
      parts[i].z += parts[i].vz * dt;
      // Decay life
      parts[i].life -= 0.2f * dt;

      // Remove dead particles or those below floor
      if (parts[i].y < -7.5f || parts[i].life <= 0.0f) {
        continue;
      }

      // Draw particle
      Vector3 pos = Vector3(parts[i].x, parts[i].y, parts[i].z);
      float b = parts[i].life;
      if (b < 0.15f) b = 0.15f;
      Color c = Color(parts[i].hue, RainbowGradientPalette);
      c.scale((uint8_t)(b * brightness));
      voxel(pos, c);
      parts[alive] = parts[i];
      alive++;
    }
    numParts = alive;
  }
};
#endif
