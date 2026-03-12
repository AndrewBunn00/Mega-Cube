#ifndef MAZE_H
#define MAZE_H

#include <string.h>
#include "Animation.h"

class Maze : public Animation {
 private:
  static const int SZ = 8;
  // 6 directions: +x, -x, +y, -y, +z, -z
  bool walls[SZ][SZ][SZ][6];
  bool visited[SZ][SZ][SZ];

  struct Pos { int8_t x, y, z; };

  Pos path[512];
  int pathLen;
  float pathProgress;
  float timer;
  float doneTimer;

  // DFS stack for generation
  Pos stack[512];
  int stackTop;

  static constexpr auto &settings = config.animation.maze;

  // Direction offsets: +x, -x, +y, -y, +z, -z
  int dx(int d) { int t[] = {1,-1,0,0,0,0}; return t[d]; }
  int dy(int d) { int t[] = {0,0,1,-1,0,0}; return t[d]; }
  int dz(int d) { int t[] = {0,0,0,0,1,-1}; return t[d]; }
  int opposite(int d) { int t[] = {1,0,3,2,5,4}; return t[d]; }

  bool inBounds(int x, int y, int z) {
    return x >= 0 && x < SZ && y >= 0 && y < SZ && z >= 0 && z < SZ;
  }

  void generate() {
    // Initialize all walls to true
    memset(walls, 1, sizeof(walls));
    memset(visited, 0, sizeof(visited));

    // DFS maze generation starting from (0,0,0)
    stackTop = 0;
    stack[stackTop++] = {0, 0, 0};
    visited[0][0][0] = true;

    while (stackTop > 0) {
      Pos cur = stack[stackTop - 1];

      // Find unvisited neighbors
      int neighbors[6];
      int nv = 0;
      for (int d = 0; d < 6; d++) {
        int nx = cur.x + dx(d);
        int ny = cur.y + dy(d);
        int nz = cur.z + dz(d);
        if (inBounds(nx, ny, nz) && !visited[nx][ny][nz]) {
          neighbors[nv++] = d;
        }
      }

      if (nv > 0) {
        // Pick random neighbor
        int pick = (int)noise.nextRandom(0, nv);
        if (pick >= nv) pick = nv - 1;
        int d = neighbors[pick];
        int nx = cur.x + dx(d);
        int ny = cur.y + dy(d);
        int nz = cur.z + dz(d);

        // Remove walls between cur and neighbor
        walls[cur.x][cur.y][cur.z][d] = false;
        walls[nx][ny][nz][opposite(d)] = false;

        visited[nx][ny][nz] = true;
        stack[stackTop++] = {(int8_t)nx, (int8_t)ny, (int8_t)nz};
      } else {
        stackTop--;
      }
    }
  }

  void solve() {
    // BFS from (0,0,0) to (SZ-1, SZ-1, SZ-1)
    memset(visited, 0, sizeof(visited));

    // BFS queue and parent tracking
    Pos queue[512];
    int parent[SZ][SZ][SZ];
    int parentDir[SZ][SZ][SZ];
    memset(parent, -1, sizeof(parent));

    int qHead = 0, qTail = 0;
    queue[qTail++] = {0, 0, 0};
    visited[0][0][0] = true;

    bool found = false;
    while (qHead < qTail && !found) {
      Pos cur = queue[qHead++];
      if (cur.x == SZ - 1 && cur.y == SZ - 1 && cur.z == SZ - 1) {
        found = true;
        break;
      }
      for (int d = 0; d < 6; d++) {
        if (walls[cur.x][cur.y][cur.z][d]) continue;
        int nx = cur.x + dx(d);
        int ny = cur.y + dy(d);
        int nz = cur.z + dz(d);
        if (!inBounds(nx, ny, nz)) continue;
        if (visited[nx][ny][nz]) continue;
        visited[nx][ny][nz] = true;
        parent[nx][ny][nz] = cur.x * SZ * SZ + cur.y * SZ + cur.z;
        parentDir[nx][ny][nz] = d;
        queue[qTail++] = {(int8_t)nx, (int8_t)ny, (int8_t)nz};
      }
    }

    // Reconstruct path
    pathLen = 0;
    if (found) {
      Pos cur = {(int8_t)(SZ - 1), (int8_t)(SZ - 1), (int8_t)(SZ - 1)};
      while (!(cur.x == 0 && cur.y == 0 && cur.z == 0)) {
        path[pathLen++] = cur;
        int p = parent[cur.x][cur.y][cur.z];
        cur.x = p / (SZ * SZ);
        cur.y = (p / SZ) % SZ;
        cur.z = p % SZ;
      }
      path[pathLen++] = {0, 0, 0};
      // Reverse path
      for (int i = 0; i < pathLen / 2; i++) {
        Pos tmp = path[i];
        path[i] = path[pathLen - 1 - i];
        path[pathLen - 1 - i] = tmp;
      }
    }
  }

 public:
  void init() {
    state = state_t::STARTING;
    timer_starting = settings.starttime;
    timer_running = settings.runtime;
    timer_ending = settings.endtime;
    generate();
    solve();
    pathProgress = 0;
    timer = 0;
    doneTimer = 0;
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

    timer += dt;

    // Advance solve progress
    float solveSpeed = 4.0f;  // cells per second
    if (pathProgress < pathLen) {
      pathProgress += solveSpeed * dt;
      doneTimer = 0;
    } else {
      doneTimer += dt;
      if (doneTimer > 3.0f) {
        // Generate new maze
        generate();
        solve();
        pathProgress = 0;
        doneTimer = 0;
      }
    }

    // Draw maze walls as dim voxels (2x2x2 blocks per cell)
    for (int cx = 0; cx < SZ; cx++)
      for (int cy = 0; cy < SZ; cy++)
        for (int cz = 0; cz < SZ; cz++) {
          // Draw cell center as dim block
          float bx = cx * 2.0f - 7.0f;
          float by = cy * 2.0f - 7.0f;
          float bz = cz * 2.0f - 7.0f;
          Color wallColor = Color(40, 40, 80).scaled(brightness);
          for (int ox = 0; ox < 2; ox++)
            for (int oy = 0; oy < 2; oy++)
              for (int oz = 0; oz < 2; oz++)
                voxel(Vector3(bx + ox, by + oy, bz + oz), wallColor);
        }

    // Draw path cells brighter
    int showCells = (int)pathProgress;
    if (showCells > pathLen) showCells = pathLen;
    for (int i = 0; i < showCells; i++) {
      float bx = path[i].x * 2.0f - 7.0f;
      float by = path[i].y * 2.0f - 7.0f;
      float bz = path[i].z * 2.0f - 7.0f;
      float frac = (float)i / pathLen;
      Color pathColor = Color((uint8_t)(frac * 255), RainbowGradientPalette);
      pathColor.scale(brightness);
      for (int ox = 0; ox < 2; ox++)
        for (int oy = 0; oy < 2; oy++)
          for (int oz = 0; oz < 2; oz++)
            voxel(Vector3(bx + ox, by + oy, bz + oz), pathColor);
    }

    // Glow at solve head
    if (showCells > 0 && showCells <= pathLen) {
      int headCell = showCells - 1;
      float bx = path[headCell].x * 2.0f - 7.0f + 0.5f;
      float by = path[headCell].y * 2.0f - 7.0f + 0.5f;
      float bz = path[headCell].z * 2.0f - 7.0f + 0.5f;
      radiate(Vector3(bx, by, bz), Color::WHITE.scaled(brightness), 2.5f);
    }

    // Start marker glow (green)
    radiate(Vector3(-7.0f + 0.5f, -7.0f + 0.5f, -7.0f + 0.5f),
            Color::GREEN.scaled(brightness), 2.0f);
    // End marker glow (red)
    radiate(Vector3(7.0f + 0.5f, 7.0f + 0.5f, 7.0f + 0.5f),
            Color::RED.scaled(brightness), 2.0f);
  }
};
#endif
