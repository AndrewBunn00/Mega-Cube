#ifndef RAIN_H
#define RAIN_H

#include "Animation.h"

class Rain : public Animation {
 private:
  struct Drop { float x, y, z, speed; bool active; };
  struct Splash { float x, y, z, vx, vy, vz, life; };
  static const int MAX_DROPS = 80;
  static const int MAX_SPLASHES = 150;
  Drop drops[MAX_DROPS];
  Splash splashes[MAX_SPLASHES];
  int numSplashes = 0;
  float spawnTimer = 0;

  static constexpr auto &settings = config.animation.rain;

 public:
  void init() {
    state = state_t::STARTING;
    timer_starting = settings.starttime;
    timer_running = settings.runtime;
    timer_ending = settings.endtime;
    for (int i = 0; i < MAX_DROPS; i++) drops[i].active = false;
    numSplashes = 0;
    spawnTimer = 0;
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

    // spawn drops
    spawnTimer += dt;
    if (spawnTimer > 0.02f) {
      spawnTimer = 0;
      for (int i = 0; i < MAX_DROPS; i++) {
        if (!drops[i].active) {
          drops[i].x = noise.nextRandom(-7.5f, 7.5f);
          drops[i].z = noise.nextRandom(-7.5f, 7.5f);
          drops[i].y = 7.5f;
          drops[i].speed = noise.nextRandom(15, 25);
          drops[i].active = true;
          break;
        }
      }
    }

    // move drops
    for (int i = 0; i < MAX_DROPS; i++) {
      if (!drops[i].active) continue;
      drops[i].y -= drops[i].speed * dt;
      if (drops[i].y <= -7.5f) {
        for (int j = 0; j < 4 && numSplashes < MAX_SPLASHES; j++) {
          Splash& s = splashes[numSplashes++];
          s.x = drops[i].x; s.y = -7.5f; s.z = drops[i].z;
          s.vx = noise.nextRandom(-3, 3);
          s.vy = noise.nextRandom(3, 8);
          s.vz = noise.nextRandom(-3, 3);
          s.life = 1.0f;
        }
        drops[i].active = false;
      } else {
        voxel(Vector3(drops[i].x, drops[i].y, drops[i].z), Color(80, 130, 255));
      }
    }

    // splashes
    for (int i = 0; i < numSplashes; ) {
      Splash& s = splashes[i];
      s.x += s.vx * dt; s.y += s.vy * dt; s.z += s.vz * dt;
      s.vy -= 20.0f * dt;
      s.life -= dt * 2.5f;
      if (s.life <= 0 || s.y < -7.5f) {
        splashes[i] = splashes[--numSplashes];
      } else {
        Color c(150, 200, 255);
        c.scale((uint8_t)(s.life * 255));
        voxel(Vector3(s.x, s.y, s.z), c);
        i++;
      }
    }
  }
};
#endif
