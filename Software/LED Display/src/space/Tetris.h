#ifndef TETRIS_H
#define TETRIS_H

#include <string.h>
#include "Animation.h"

class Tetris : public Animation {
 private:
  bool board[16][16][16];
  Color boardColors[16][16][16];

  struct Piece {
    int8_t cells[4][3];  // up to 4 cells, each with x,y,z offset
    int8_t numCells;
  };

  static const int NUM_TYPES = 5;
  Piece types[NUM_TYPES];

  int pieceX, pieceY, pieceZ;
  Piece cur;
  Color pieceColor;
  float fallTimer;
  float fallInterval = 0.25f;

  static constexpr auto &settings = config.animation.tetris;

  void initTypes() {
    // I-piece (vertical bar of 4)
    types[0] = {{{0,0,0},{0,1,0},{0,2,0},{0,3,0}}, 4};
    // L-piece
    types[1] = {{{0,0,0},{0,1,0},{0,2,0},{1,2,0}}, 4};
    // T-piece
    types[2] = {{{0,0,0},{1,0,0},{2,0,0},{1,1,0}}, 4};
    // S-piece
    types[3] = {{{0,0,0},{1,0,0},{1,1,0},{2,1,0}}, 4};
    // O-piece (2x2 block)
    types[4] = {{{0,0,0},{1,0,0},{0,0,1},{1,0,1}}, 4};
  }

  bool fits(int px, int py, int pz, const Piece &p) {
    for (int i = 0; i < p.numCells; i++) {
      int x = px + p.cells[i][0];
      int y = py + p.cells[i][1];
      int z = pz + p.cells[i][2];
      if (x < 0 || x > 15 || y < 0 || y > 15 || z < 0 || z > 15) return false;
      if (board[x][y][z]) return false;
    }
    return true;
  }

  void lock(int px, int py, int pz, const Piece &p, Color c) {
    for (int i = 0; i < p.numCells; i++) {
      int x = px + p.cells[i][0];
      int y = py + p.cells[i][1];
      int z = pz + p.cells[i][2];
      if (x >= 0 && x < 16 && y >= 0 && y < 16 && z >= 0 && z < 16) {
        board[x][y][z] = true;
        boardColors[x][y][z] = c;
      }
    }
  }

  void clearLayers() {
    for (int y = 15; y >= 0; y--) {
      bool full = true;
      for (int x = 0; x < 16 && full; x++)
        for (int z = 0; z < 16 && full; z++)
          if (!board[x][y][z]) full = false;
      if (full) {
        // Shift everything above down
        for (int yy = y; yy < 15; yy++)
          for (int x = 0; x < 16; x++)
            for (int z = 0; z < 16; z++) {
              board[x][yy][z] = board[x][yy + 1][z];
              boardColors[x][yy][z] = boardColors[x][yy + 1][z];
            }
        // Clear top layer
        for (int x = 0; x < 16; x++)
          for (int z = 0; z < 16; z++) {
            board[x][15][z] = false;
            boardColors[x][15][z] = Color::BLACK;
          }
        y++;  // re-check this layer
      }
    }
  }

  void spawn() {
    int type = (int)noise.nextRandom(0, NUM_TYPES);
    if (type >= NUM_TYPES) type = NUM_TYPES - 1;
    cur = types[type];
    pieceX = (int)noise.nextRandom(2, 12);
    if (pieceX > 12) pieceX = 12;
    pieceZ = (int)noise.nextRandom(2, 12);
    if (pieceZ > 12) pieceZ = 12;
    pieceY = 15;
    uint8_t hue = (uint8_t)noise.nextRandom(0, 256);
    pieceColor = Color(hue, RainbowGradientPalette);
    fallTimer = 0;

    // If the piece doesn't fit at spawn, clear the board (game over)
    if (!fits(pieceX, pieceY, pieceZ, cur)) {
      memset(board, 0, sizeof(board));
      memset(boardColors, 0, sizeof(boardColors));
    }
  }

 public:
  void init() {
    state = state_t::STARTING;
    timer_starting = settings.starttime;
    timer_running = settings.runtime;
    timer_ending = settings.endtime;
    memset(board, 0, sizeof(board));
    memset(boardColors, 0, sizeof(boardColors));
    initTypes();
    spawn();
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

    fallTimer += dt;
    if (fallTimer >= fallInterval) {
      fallTimer -= fallInterval;

      // AI: try to move piece toward center-bottom, try random horizontal moves
      int bestX = pieceX;
      int bestZ = pieceZ;

      // Try a random horizontal move toward a better position
      int dx = 0, dz = 0;
      int rdir = (int)noise.nextRandom(0, 4);
      if (rdir >= 4) rdir = 3;
      if (rdir == 0) dx = 1;
      else if (rdir == 1) dx = -1;
      else if (rdir == 2) dz = 1;
      else dz = -1;

      if (fits(pieceX + dx, pieceY, pieceZ + dz, cur)) {
        pieceX += dx;
        pieceZ += dz;
      }

      // Try to fall
      if (fits(pieceX, pieceY - 1, pieceZ, cur)) {
        pieceY--;
      } else {
        // Lock piece
        lock(pieceX, pieceY, pieceZ, cur, pieceColor);
        clearLayers();
        spawn();
      }
    }

    // Draw the board
    for (int x = 0; x < 16; x++)
      for (int y = 0; y < 16; y++)
        for (int z = 0; z < 16; z++) {
          if (board[x][y][z]) {
            Vector3 pos(x - 7.5f, y - 7.5f, z - 7.5f);
            voxel(pos, boardColors[x][y][z].scaled(brightness));
          }
        }

    // Draw the falling piece
    for (int i = 0; i < cur.numCells; i++) {
      int x = pieceX + cur.cells[i][0];
      int y = pieceY + cur.cells[i][1];
      int z = pieceZ + cur.cells[i][2];
      if (x >= 0 && x < 16 && y >= 0 && y < 16 && z >= 0 && z < 16) {
        Vector3 pos(x - 7.5f, y - 7.5f, z - 7.5f);
        voxel(pos, pieceColor.scaled(brightness));
      }
    }
  }
};
#endif
