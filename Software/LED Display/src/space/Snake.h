#ifndef SNAKE_H
#define SNAKE_H

#include <string.h>
#include "Animation.h"

class Snake : public Animation {
 private:
  static const int MAX_LEN = 200;
  struct Pos { int8_t x, y, z; };
  Pos body[MAX_LEN];
  int length;
  int headIdx;
  Pos food;
  float moveTimer;
  float moveInterval = 0.1f;
  bool grid[16][16][16];

  static constexpr auto &settings = config.animation.snake;

  void placeFood() {
    // Find a free cell for the food
    for (int attempts = 0; attempts < 500; attempts++) {
      food.x = (int)noise.nextRandom(0, 16);
      food.y = (int)noise.nextRandom(0, 16);
      food.z = (int)noise.nextRandom(0, 16);
      if (food.x > 15) food.x = 15;
      if (food.y > 15) food.y = 15;
      if (food.z > 15) food.z = 15;
      if (!grid[food.x][food.y][food.z]) return;
    }
  }

  bool isValid(int x, int y, int z) {
    return x >= 0 && x < 16 && y >= 0 && y < 16 && z >= 0 && z < 16;
  }

  void resetGame() {
    memset(grid, 0, sizeof(grid));
    length = 3;
    headIdx = 2;
    for (int i = 0; i < length; i++) {
      body[i].x = 8;
      body[i].y = 8;
      body[i].z = 8 + i;
      grid[body[i].x][body[i].y][body[i].z] = true;
    }
    placeFood();
    moveTimer = 0;
    hue16 = 0;
  }

 public:
  void init() {
    state = state_t::STARTING;
    timer_starting = settings.starttime;
    timer_running = settings.runtime;
    timer_ending = settings.endtime;
    resetGame();
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

    moveTimer += dt;
    if (moveTimer >= moveInterval) {
      moveTimer -= moveInterval;
      hue16 += 256;

      Pos head = body[headIdx];

      // AI: determine direction toward food
      // Try the 6 possible directions, prefer the one closest to food
      int dx[6] = {1, -1, 0, 0, 0, 0};
      int dy[6] = {0, 0, 1, -1, 0, 0};
      int dz[6] = {0, 0, 0, 0, 1, -1};

      int bestDir = -1;
      float bestDist = 1e9f;

      for (int d = 0; d < 6; d++) {
        int nx = head.x + dx[d];
        int ny = head.y + dy[d];
        int nz = head.z + dz[d];
        if (!isValid(nx, ny, nz)) continue;
        if (grid[nx][ny][nz]) continue;
        float dist = (float)((nx - food.x) * (nx - food.x) +
                             (ny - food.y) * (ny - food.y) +
                             (nz - food.z) * (nz - food.z));
        if (dist < bestDist) {
          bestDist = dist;
          bestDir = d;
        }
      }

      // If no best direction toward food, pick any valid direction
      if (bestDir < 0) {
        for (int d = 0; d < 6; d++) {
          int nx = head.x + dx[d];
          int ny = head.y + dy[d];
          int nz = head.z + dz[d];
          if (isValid(nx, ny, nz) && !grid[nx][ny][nz]) {
            bestDir = d;
            break;
          }
        }
      }

      // If still no valid move, reset the game
      if (bestDir < 0) {
        resetGame();
      } else {
        Pos newHead;
        newHead.x = head.x + dx[bestDir];
        newHead.y = head.y + dy[bestDir];
        newHead.z = head.z + dz[bestDir];

        bool ate = (newHead.x == food.x && newHead.y == food.y && newHead.z == food.z);

        if (!ate) {
          // Remove tail
          int tailIdx = (headIdx - length + 1 + MAX_LEN) % MAX_LEN;
          grid[body[tailIdx].x][body[tailIdx].y][body[tailIdx].z] = false;
        } else {
          if (length < MAX_LEN) length++;
          placeFood();
        }

        headIdx = (headIdx + 1) % MAX_LEN;
        body[headIdx] = newHead;
        grid[newHead.x][newHead.y][newHead.z] = true;
      }
    }

    // Draw body with rainbow colors
    for (int i = 0; i < length; i++) {
      int idx = (headIdx - length + 1 + i + MAX_LEN) % MAX_LEN;
      float frac = (float)i / length;
      uint8_t hue = (uint8_t)((hue16 >> 8) + (int)(frac * 128));
      Color c = Color(hue, RainbowGradientPalette);
      c.scale(brightness);
      Vector3 pos(body[idx].x - 7.5f, body[idx].y - 7.5f, body[idx].z - 7.5f);
      voxel(pos, c);
    }

    // Draw blinking food
    float blink = sinf(moveTimer * 20.0f);
    if (blink > 0) {
      Vector3 fpos(food.x - 7.5f, food.y - 7.5f, food.z - 7.5f);
      radiate(fpos, Color::WHITE.scaled(brightness), 1.5f);
    }
  }
};
#endif
