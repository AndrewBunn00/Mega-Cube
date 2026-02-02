# MEGA CUBE - Math Reference

Quick reference for the mathematical concepts used in the cube software.

## Coordinate Systems

Two coordinate systems are used throughout:

```
Physical [0-15]:          System [-7.5, 7.5]:
Array indices             Math operations

    15                         7.5
    │                           │
    │     cube[x][y][z]         │    Vector3(x,y,z)
    │                           │
    0 ────                  -7.5 ──── 0 ────── 7.5
```

**Convert**: `physical = round(system + 7.5)` or `system = physical - 7.5`

**Constants**: `CX = CY = CZ = 7.5` (center point)

---

## Vector3 Math

### Key Operations

```cpp
Vector3 a, b;

// Dot product: projects a onto b
float projection = a.dot(b);  // = ax*bx + ay*by + az*bz = |a||b|cos(θ)

// Cross product: perpendicular vector
Vector3 perpendicular = a.cross(b);  // Right-hand rule, |a×b| = |a||b|sin(θ)

// Normalize: unit vector
a.normalize();  // Same direction, length = 1
```

### Rodrigues' Rotation Formula

Rotates vector `v` around axis `k` by angle `θ`:

```
v' = v·cos(θ) + (k × v)·sin(θ) + k·(k·v)·(1 - cos(θ))
   └─────┬─────┘ └──────┬────────┘ └────────┬──────────┘
     parallel      perpendicular       axis
   unchanged      rotates in plane   component
```

See [Math3D.cpp:34-41](Mega-Cube/Software/LED Display/src/power/Math3D.cpp#L34-L41)

---

## Quaternions

### Why Quaternions?

Euler angles suffer from **gimbal lock** (lose 1 DOF when axes align). Quaternions solve this.

### Structure

```
q = w + xi + yj + zk
    │   └────┬────┘
  scalar  vector (imaginary)
```

### Axis-Angle Construction

To rotate by θ around unit axis **k**:

```cpp
q.w = cos(θ/2)
q.v = sin(θ/2) * k
```

**Note the half-angle!** This is key to quaternion rotations.

### Rotating a Vector

```
v' = q·v·q⁻¹
```

Where `q⁻¹ = (w, -x, -y, -z)` for unit quaternions.

**Visual**:
```
      q         v        q⁻¹
   ┌────┐    ┌───┐    ┌─────┐
   │θ/2 │ * │ v │ *  │-θ/2 │ = rotated v
   │ k  │   └───┘    │ -k  │
   └────┘            └─────┘
```

### Composition

Multiply quaternions to combine rotations:

```cpp
q_combined = q2 * q1;  // Apply q1 first, then q2
v_rotated = q_combined.rotate(v);
```

**Formula** for `q1 * q2`:
```
w_result = w₁w₂ - v₁·v₂
v_result = w₁v₂ + w₂v₁ + v₁×v₂
```

See [Math3D.cpp:73-87](Mega-Cube/Software/LED Display/src/power/Math3D.cpp#L73-L87)

---

## Perlin Noise

### What It Does

Generates smooth, continuous pseudo-random values. Same input → same output.

```
Pure Random:        Perlin Noise:
█ ▁█▆ ▃█ ▂          ╱‾╲  ╱‾╲
 █  ▃  ▇ █         ╱    ╲╱    ╲
```

### Algorithm Overview

1. **Grid vertices** have random gradient vectors
2. **Interpolate** using smooth fade function: `f(t) = 6t⁵ - 15t⁴ + 10t³`
3. **Dot products** between position and gradients

### 4D Noise

Used in Plasma animation - samples (x, y, z, time) for organic morphing:

```cpp
noise.noise4(x, y, z, time_offset)
```

As `time_offset` changes, the noise field "drifts" through 4D space.

See [Noise.cpp:115-178](Mega-Cube/Software/LED Display/src/power/Noise.cpp#L115-L178)

---

## Color Math

### Palette Interpolation

Palettes are `[index, R, G, B, ...]` arrays. To get color at arbitrary index:

```
Palette: [0,255,0,0,  128,0,255,0,  255,0,0,255]
           Red         Green          Blue

Index 64: Linearly interpolate between Red(0) and Green(128)
  → blend = 64/128 = 0.5
  → color = Red * 0.5 + Green * 0.5
```

See [Color.cpp:29-60](Mega-Cube/Software/LED Display/src/power/Color.cpp#L29-L60)

### Gamma Correction

Human eye perceives brightness non-linearly. Gamma curve (γ ≈ 2.2) compensates:

```
Linear:     0   64  128  192  255
Perceived:  0   5   21   66   255
```

Lookup table `gamma8[]` maps input → corrected output.

---

## Particle Physics

### Euler Integration

Basic physics simulation:

```cpp
// Update velocity with acceleration
velocity += gravity * dt;

// Update position with velocity
position += velocity * dt;
```

**Note**: First-order Euler is simple but less accurate for large `dt`. Good enough for 30 FPS.

### Fireworks Example

```
Launch:  v = (0, +speed, 0)
         a = (0, -9.8, 0)  // gravity

Explode: 200 particles with random velocities
         Each affected by gravity
         Fade based on time
```

See [Fireworks.cpp:67-94](Mega-Cube/Software/LED Display/src/space/Fireworks.cpp#L67-L94)

---

## Perspective Projection

Starfield animation uses perspective to create depth:

```
     Far                Near
      *        →         ✱
       *                ✹
        *              ✺

Scale = f / (f + z)

Where f = focal length (body_diagonal parameter)
      z = distance from camera
```

Closer stars → larger scale → appear bigger and brighter.

See [Starfield.cpp:48-62](Mega-Cube/Software/LED Display/src/space/Starfield.cpp#L48-L62)

---

## Fast Math Tricks

### Bitwise Boundary Check

Instead of:
```cpp
if (x >= 0 && x < 16 && y >= 0 && y < 16 && z >= 0 && z < 16)
```

Use:
```cpp
if (((x | y | z) & 0xFFF0) == 0)
```

**Why it works**: Valid range is [0, 15] (4 bits). OR all coords; if any bit 4+ is set, out of bounds.

### 8-bit Blending

```cpp
// Integer-only blend: result = a*(1-t) + b*t
uint8_t blend8(uint8_t t, uint8_t a, uint8_t b) {
  uint16_t partial = a * (255 - t) + b * t + 128;  // +128 for rounding
  return partial >> 8;
}
```

Avoids floating-point math.

### Fixed-Point Hue

```cpp
int16_t hue16;  // 16-bit hue
hue16 += speed * dt * 256;  // High precision
uint8_t hue = hue16 >> 8;   // Extract actual hue
```

Smooth color rotation without jitter.

---

## Key Math Files

- [Math3D.cpp](Mega-Cube/Software/LED Display/src/power/Math3D.cpp) - Vector3, Quaternion
- [Noise.cpp](Mega-Cube/Software/LED Display/src/power/Noise.cpp) - Perlin noise
- [Color.cpp](Mega-Cube/Software/LED Display/src/power/Color.cpp) - Palette interpolation
- [Math8.h](Mega-Cube/Software/LED Display/src/power/Math8.h) - Fast 8-bit ops
- [Particle.cpp](Mega-Cube/Software/LED Display/src/power/Particle.cpp) - Physics integration
