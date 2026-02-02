# MEGA CUBE - Animations Reference

## Animation System

All animations inherit from base `Animation` class and cycle through:

```
INACTIVE → STARTING → RUNNING → ENDING → INACTIVE
          (fade in)   (main)   (fade out)
```

Each implements:
- `init()` - Load config and reset state
- `draw(float dt)` - Render one frame (dt = delta time in seconds)

## Built-in Animations

### 1. Atoms
**File**: [Atoms.cpp](Mega-Cube/Software/LED Display/src/space/Atoms.cpp)

9 electrons orbiting nucleus on **dynamically changing axes**:

```cpp
// Axis evolves over time using different frequencies per electron
Vector3 axis(
  cos(time * TWO_PI + i * 0.7),
  sin(time * TWO_PI + i * 1.3),
  sin(time * TWO_PI + i * 2.1)
);
Quaternion rotation(angle_speed * dt, axis);
electron[i] = rotation.rotate(electron[i]);
```

Creates chaotic but beautiful orbital patterns.

---

### 2. Sinus
**File**: [Sinus.cpp](Mega-Cube/Software/LED Display/src/space/Sinus.cpp)

3D wave surface: `z = A·sin(√(x² + y²) + ωt)`

Concentric ripples emanating from center.

---

### 3. Starfield
**File**: [Starfield.cpp](Mega-Cube/Software/LED Display/src/space/Starfield.cpp)

200 stars with **perspective projection**:

```cpp
scale = body_diagonal / (body_diagonal + distance);
projected = star.position * scale;
brightness = 1 - (distance / body_diagonal);
```

Classic "warp speed" effect.

---

### 4. Fireworks
**File**: [Fireworks.cpp](Mega-Cube/Software/LED Display/src/space/Fireworks.cpp)

Two-phase particle system:

1. **Missile**: Rises with gravity until peak
2. **Explosion**: 200 debris particles with random velocities, gravity, fade over time

```cpp
debris[i].position = missile.position;
debris[i].velocity = random_sphere_vector() * explosion_speed;
// Each particle affected by gravity, fades based on time
```

---

### 5. Twinkels
**File**: [Twinkels.cpp](Mega-Cube/Software/LED Display/src/space/Twinkels.cpp)

Random twinkling using **Gaussian fade curve**:

```cpp
intensity = exp(-pow(t - 0.5, 2) / 0.1);  // Peak at t=0.5
```

Each voxel has randomized timing for natural effect.

---

### 6. Helix
**File**: [Helix.cpp](Mega-Cube/Software/LED Display/src/space/Helix.cpp)

DNA double helix via parametric curves:

```cpp
// Helix 1
p1 = (r·cos(t), pitch·t, r·sin(t))
// Helix 2 (180° phase shift)
p2 = (r·cos(t+π), pitch·t, r·sin(t+π))
```

Both rotated via quaternions.

---

### 7. Arrows
**File**: [Arrows.cpp](Mega-Cube/Software/LED Display/src/space/Arrows.cpp)

Spiral of rotating arrow shapes on helical path.

---

### 8. Plasma
**File**: [Plasma.cpp](Mega-Cube/Software/LED Display/src/space/Plasma.cpp)

**4D Perlin noise** for organic volumetric effects:

```cpp
value = noise.noise4(x*scale, y*scale, z*scale, time_offset);
noise_map[x][y][z] = (value + 1.0) * 127.5;  // Map [-1,1] to [0,255]
```

Drifting through 4D space creates morphing clouds.

---

### 9. Mario
**File**: [Mario.cpp](Mega-Cube/Software/LED Display/src/space/Mario.cpp)

2D sprite art mapped to 3D voxel planes. Character data from `gfx/` directory.

---

### 10. Life
**File**: [Life.cpp](Mega-Cube/Software/LED Display/src/space/Life.cpp)

3D Conway's Game of Life (26-neighbor rules):

- **Birth**: Dead cell with 4-5 living neighbors
- **Survival**: Living cell with 4-6 living neighbors

Hash-based cycle detection prevents infinite loops.

---

### 11. Pong
**File**: [Pong.cpp](Mega-Cube/Software/LED Display/src/space/Pong.cpp)

3D Pong simulation with ball physics and paddle tracking.

---

### 12. Spectrum
**File**: [Spectrum.cpp](Mega-Cube/Software/LED Display/src/space/Spectrum.cpp)

Audio FFT visualization (64 frequency bins from ESP8266). Currently stubbed.

---

### 13. Scroller
**File**: [Scroller.cpp](Mega-Cube/Software/LED Display/src/space/Scroller.cpp)

Text on **cylindrical projection**:

```cpp
float angle = scroll_offset + i * angle_per_char;
float x = radius * cos(angle);
float z = radius * sin(angle);
render_char(text[i], x, 0, z, angle);
```

Creates marquee sign effect.

---

### 14. Accelerometer
**File**: [Accelerometer.cpp](Mega-Cube/Software/LED Display/src/space/Accelerometer.cpp)

Particles fall in direction of gravity vector from IMU sensor. Interactive - tilt cube to control flow.

---

### 15. Cube
**File**: [Cube.cpp](Mega-Cube/Software/LED Display/src/space/Cube.cpp)

Wireframe cube (8 vertices, 12 edges) with quaternion rotations on X, Y, Z axes.

---

### 16. Countdown
**File**: [Countdown.cpp](Mega-Cube/Software/LED Display/src/space/Countdown.cpp)

Large numeric countdown with optional explosion effect at zero.

---

## Creating Custom Animations

### Minimal Template

```cpp
// MyAnimation.h
#pragma once
#include "../core/Animation.h"

class MyAnimation : public Animation {
public:
  void init() override;
  void draw(float dt) override;
private:
  Vector3 position;
  float phase;
};

extern MyAnimation myAnimation;
```

```cpp
// MyAnimation.cpp
#include "MyAnimation.h"
#include "../core/Config.h"
#include "../core/Graphics.h"

MyAnimation myAnimation;

void MyAnimation::init() {
  timer_starting.time_alarm = config.animation.myAnimation.starttime * 1000000;
  timer_running.time_alarm = config.animation.myAnimation.runtime * 1000000;
  timer_ending.time_alarm = config.animation.myAnimation.endtime * 1000000;

  state = state_t::STARTING;
  timer_starting.reset();

  position = Vector3(0, 0, 0);
  phase = 0;
}

void MyAnimation::draw(float dt) {
  // Handle state transitions (standard pattern)
  switch (state) {
    case state_t::STARTING:
      brightness = 255 * timer_starting.ratio();
      if (timer_starting.update()) {
        state = state_t::RUNNING;
        timer_running.reset();
      }
      break;
    case state_t::RUNNING:
      brightness = 255;
      if (timer_running.update()) {
        state = state_t::ENDING;
        timer_ending.reset();
      }
      break;
    case state_t::ENDING:
      brightness = 255 * (1 - timer_ending.ratio());
      if (timer_ending.update()) {
        state = state_t::INACTIVE;
      }
      break;
  }

  // Your animation logic
  phase += config.animation.myAnimation.speed * dt;
  float radius = 3.0 + 2.0 * sin(phase);
  radiate(position, Color(hue16 >> 8, RainbowGradient), radius);

  hue16 += config.animation.myAnimation.hue_speed * dt * 256;
}
```

### Add to Config.h

```cpp
struct {
  float starttime = 1.0;
  float runtime = 15.0;
  float endtime = 1.0;
  float speed = 2.0;
  float hue_speed = 1.0;
  uint8_t brightness = 255;
  uint8_t motionBlur = 64;
} myAnimation;
```

### Register in Animation.cpp

```cpp
#include "../space/MyAnimation.h"

// In get_item() array:
{&myAnimation, nullptr},
```

## Common Patterns

### Particles
```cpp
Particle particles[100];
for (int i = 0; i < 100; i++) {
  particles[i].move(dt, gravity);
  radiate(particles[i].position, color, 1.0);
}
```

### Rotation
```cpp
Quaternion rotation(speed * dt, axis);
for (int i = 0; i < count; i++) {
  points[i] = rotation.rotate(points[i]);
  voxel(points[i], color);
}
```

### Noise
```cpp
offset += speed * dt;
for x, y, z:
  value = noise.noise4(x * scale, y * scale, z * scale, offset);
  voxel(x, y, z, Color(value * 255, palette));
```

## Key Tips

- **Always use `dt`** for time-based calculations (frame-rate independence)
- **16-bit hue** for smooth palette rotation: `hue16 += speed * dt * 256`
- **Boundary checks** handled by `voxel()` functions automatically
- **Avoid dynamic allocation** - use static arrays
- **Config parameters** make animations tunable without recompiling
