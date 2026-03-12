#ifndef BOIDS_H
#define BOIDS_H

#include "Animation.h"

class Boids : public Animation {
 private:
  static const int NUM_BOIDS = 50;

  struct Boid {
    float x, y, z;
    float vx, vy, vz;
    uint8_t hue;
  };

  Boid boids[NUM_BOIDS];

  static constexpr auto &settings = config.animation.boids;

 public:
  void init() {
    state = state_t::STARTING;
    timer_starting = settings.starttime;
    timer_running = settings.runtime;
    timer_ending = settings.endtime;

    for (int i = 0; i < NUM_BOIDS; i++) {
      boids[i].x = noise.nextRandom(-5.0f, 5.0f);
      boids[i].y = noise.nextRandom(-5.0f, 5.0f);
      boids[i].z = noise.nextRandom(-5.0f, 5.0f);
      boids[i].vx = noise.nextRandom(-1.0f, 1.0f);
      boids[i].vy = noise.nextRandom(-1.0f, 1.0f);
      boids[i].vz = noise.nextRandom(-1.0f, 1.0f);
      boids[i].hue = (uint8_t)(int)noise.nextRandom(0, 256);
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

    // Flocking parameters
    const float cohesionWeight = 0.005f;
    const float separationWeight = 0.5f;
    const float alignmentWeight = 0.05f;
    const float separationDist = 2.5f;
    const float speedLimit = 4.0f;
    const float wallLimit = 6.5f;
    const float wallForce = 2.0f;

    for (int i = 0; i < NUM_BOIDS; i++) {
      float cohX = 0, cohY = 0, cohZ = 0;
      float sepX = 0, sepY = 0, sepZ = 0;
      float aliX = 0, aliY = 0, aliZ = 0;
      int neighbors = 0;

      for (int j = 0; j < NUM_BOIDS; j++) {
        if (i == j) continue;
        float dx = boids[j].x - boids[i].x;
        float dy = boids[j].y - boids[i].y;
        float dz = boids[j].z - boids[i].z;
        float dist = sqrtf(dx * dx + dy * dy + dz * dz);

        if (dist < 8.0f && dist > 0.001f) {
          // Cohesion: steer toward center of neighbors
          cohX += boids[j].x;
          cohY += boids[j].y;
          cohZ += boids[j].z;
          neighbors++;

          // Alignment: match velocity of neighbors
          aliX += boids[j].vx;
          aliY += boids[j].vy;
          aliZ += boids[j].vz;

          // Separation: avoid crowding
          if (dist < separationDist) {
            sepX -= dx / dist;
            sepY -= dy / dist;
            sepZ -= dz / dist;
          }
        }
      }

      if (neighbors > 0) {
        // Cohesion: steer toward average position
        cohX = (cohX / neighbors - boids[i].x) * cohesionWeight;
        cohY = (cohY / neighbors - boids[i].y) * cohesionWeight;
        cohZ = (cohZ / neighbors - boids[i].z) * cohesionWeight;

        // Alignment: steer toward average heading
        aliX = (aliX / neighbors) * alignmentWeight;
        aliY = (aliY / neighbors) * alignmentWeight;
        aliZ = (aliZ / neighbors) * alignmentWeight;
      }

      // Separation
      sepX *= separationWeight;
      sepY *= separationWeight;
      sepZ *= separationWeight;

      // Apply forces
      boids[i].vx += cohX + sepX + aliX;
      boids[i].vy += cohY + sepY + aliY;
      boids[i].vz += cohZ + sepZ + aliZ;

      // Wall avoidance: push away from boundaries
      if (boids[i].x > wallLimit) boids[i].vx -= wallForce * dt;
      if (boids[i].x < -wallLimit) boids[i].vx += wallForce * dt;
      if (boids[i].y > wallLimit) boids[i].vy -= wallForce * dt;
      if (boids[i].y < -wallLimit) boids[i].vy += wallForce * dt;
      if (boids[i].z > wallLimit) boids[i].vz -= wallForce * dt;
      if (boids[i].z < -wallLimit) boids[i].vz += wallForce * dt;

      // Speed limiting
      float speed = sqrtf(boids[i].vx * boids[i].vx +
                          boids[i].vy * boids[i].vy +
                          boids[i].vz * boids[i].vz);
      if (speed > speedLimit) {
        boids[i].vx = (boids[i].vx / speed) * speedLimit;
        boids[i].vy = (boids[i].vy / speed) * speedLimit;
        boids[i].vz = (boids[i].vz / speed) * speedLimit;
      }

      // Update position
      boids[i].x += boids[i].vx * dt;
      boids[i].y += boids[i].vy * dt;
      boids[i].z += boids[i].vz * dt;

      // Draw boid
      Vector3 pos = Vector3(boids[i].x, boids[i].y, boids[i].z);
      Color c = Color(boids[i].hue, RainbowGradientPalette);
      c.scale(brightness);
      radiate5(pos, c, 2.0f);
    }
  }
};
#endif
