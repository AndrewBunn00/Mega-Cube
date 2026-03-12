#ifndef PONG3D_H
#define PONG3D_H

#include "Animation.h"

class Pong3D : public Animation {
 private:
  float ballX, ballY, ballZ;
  float ballVX, ballVY, ballVZ;
  float p1X, p1Z;  // Player 1 paddle center (at y = -7)
  float p2X, p2Z;  // Player 2 paddle center (at y = +7)
  float padSize = 3.0f;

  static constexpr auto &settings = config.animation.pong3d;

  void resetBall() {
    ballX = 0;
    ballY = 0;
    ballZ = 0;
    float speed = 5.0f;
    ballVX = noise.nextRandom(-2.0f, 2.0f);
    ballVZ = noise.nextRandom(-2.0f, 2.0f);
    ballVY = ((int)noise.nextRandom(0, 2) == 0) ? speed : -speed;
    // Normalize and scale
    float mag = sqrtf(ballVX * ballVX + ballVY * ballVY + ballVZ * ballVZ);
    if (mag > 0) {
      ballVX = ballVX / mag * speed;
      ballVY = ballVY / mag * speed;
      ballVZ = ballVZ / mag * speed;
    }
  }

 public:
  void init() {
    state = state_t::STARTING;
    timer_starting = settings.starttime;
    timer_running = settings.runtime;
    timer_ending = settings.endtime;
    p1X = 0; p1Z = 0;
    p2X = 0; p2Z = 0;
    resetBall();
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

    // AI paddle tracking with noise wobble
    float trackSpeed = 8.0f;
    float wobble1X = noise.nextRandom(-0.5f, 0.5f);
    float wobble1Z = noise.nextRandom(-0.5f, 0.5f);
    float targetP1X = ballX + wobble1X;
    float targetP1Z = ballZ + wobble1Z;
    if (p1X < targetP1X) p1X += trackSpeed * dt;
    if (p1X > targetP1X) p1X -= trackSpeed * dt;
    if (p1Z < targetP1Z) p1Z += trackSpeed * dt;
    if (p1Z > targetP1Z) p1Z -= trackSpeed * dt;

    float wobble2X = noise.nextRandom(-0.5f, 0.5f);
    float wobble2Z = noise.nextRandom(-0.5f, 0.5f);
    float targetP2X = ballX + wobble2X;
    float targetP2Z = ballZ + wobble2Z;
    if (p2X < targetP2X) p2X += trackSpeed * dt;
    if (p2X > targetP2X) p2X -= trackSpeed * dt;
    if (p2Z < targetP2Z) p2Z += trackSpeed * dt;
    if (p2Z > targetP2Z) p2Z -= trackSpeed * dt;

    // Clamp paddles
    float limit = 7.5f - padSize / 2.0f;
    if (p1X < -limit) p1X = -limit;
    if (p1X >  limit) p1X =  limit;
    if (p1Z < -limit) p1Z = -limit;
    if (p1Z >  limit) p1Z =  limit;
    if (p2X < -limit) p2X = -limit;
    if (p2X >  limit) p2X =  limit;
    if (p2Z < -limit) p2Z = -limit;
    if (p2Z >  limit) p2Z =  limit;

    // Move ball
    ballX += ballVX * dt;
    ballY += ballVY * dt;
    ballZ += ballVZ * dt;

    // Wall bouncing (X and Z walls)
    if (ballX > 7.0f || ballX < -7.0f) { ballVX = -ballVX; ballX = (ballX > 0) ? 7.0f : -7.0f; }
    if (ballZ > 7.0f || ballZ < -7.0f) { ballVZ = -ballVZ; ballZ = (ballZ > 0) ? 7.0f : -7.0f; }

    // Paddle deflection at y boundaries
    float padY = 7.0f;
    if (ballY < -padY) {
      // Check paddle 1
      float halfPad = padSize / 2.0f;
      if (ballX > p1X - halfPad && ballX < p1X + halfPad &&
          ballZ > p1Z - halfPad && ballZ < p1Z + halfPad) {
        ballVY = -ballVY;
        ballY = -padY;
        // Deflect proportional to offset from paddle center
        float offsetX = (ballX - p1X) / halfPad;
        float offsetZ = (ballZ - p1Z) / halfPad;
        ballVX += offsetX * 3.0f;
        ballVZ += offsetZ * 3.0f;
      } else {
        resetBall();
      }
    }
    if (ballY > padY) {
      // Check paddle 2
      float halfPad = padSize / 2.0f;
      if (ballX > p2X - halfPad && ballX < p2X + halfPad &&
          ballZ > p2Z - halfPad && ballZ < p2Z + halfPad) {
        ballVY = -ballVY;
        ballY = padY;
        float offsetX = (ballX - p2X) / halfPad;
        float offsetZ = (ballZ - p2Z) / halfPad;
        ballVX += offsetX * 3.0f;
        ballVZ += offsetZ * 3.0f;
      } else {
        resetBall();
      }
    }

    // Draw paddles
    float halfPad = padSize / 2.0f;
    for (float px = -halfPad; px <= halfPad; px += 1.0f) {
      for (float pz = -halfPad; pz <= halfPad; pz += 1.0f) {
        Vector3 v1(p1X + px, -padY, p1Z + pz);
        voxel(v1, Color::BLUE.scaled(brightness));
        Vector3 v2(p2X + px, padY, p2Z + pz);
        voxel(v2, Color::RED.scaled(brightness));
      }
    }

    // Draw ball with glow
    Vector3 bpos(ballX, ballY, ballZ);
    radiate5(bpos, Color::WHITE.scaled(brightness), 2.0f);
  }
};
#endif
