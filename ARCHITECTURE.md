# MEGA CUBE - Architecture Reference

Technical details on the complex/unusual parts of the LED cube system.

## Hardware Control Pipeline

### PL9823 LED Driver

The tricky part: PL9823 LEDs expect **GRB color order** (not RGB) via serial shift registers.

```
Teensy FlexIO → Shift Registers → 4096 LEDs
   711 MHz PLL     24-bit × 4096    GRB order
```

**Key files**: [Display.cpp:97-259](Mega-Cube/Software/LED Display/src/core/Display.cpp#L97-L259)

### DMA Ping-Pong

Two DMA channels alternate to eliminate frame drops:

```
Channel 0: ████████░░░░░░░░  (transferring)
Channel 1: ░░░░░░░░████████  (being filled)
           ↓ swap when done
Channel 0: ░░░░░░░░████████  (being filled)
Channel 1: ████████░░░░░░░░  (transferring)
```

**Why**: CPU fills one buffer while DMA reads from the other. Continuous output, no CPU blocking.

### FlexIO Bit Rotation

Each RGB value → 24 DMA words (1 bit per word) for shift register loading:

```cpp
// RGB(255, 128, 0) becomes 24 DMA words:
// G: 10000000 → 8 words (0x80000000, 0, 0, 0, 0, 0, 0, 0)
// R: 11111111 → 8 words (0x80000000, 0x80000000, ...)
// B: 00000000 → 8 words (0, 0, 0, 0, 0, 0, 0, 0)

for (int bit = 7; bit >= 0; bit--) {
  channelBuffer[led++] = ((g >> bit) & 1) << 0 |
                         ((r >> bit) & 1) << 1 |
                         ((b >> bit) & 1) << 2 |
                         0xE0000000;  // Channel sync bits
}
```

See [Display.cpp:354-438](Mega-Cube/Software/LED Display/src/core/Display.cpp#L354-L438)

---

## Memory Layout

### Double Buffering with Motion Blur

```
┌─────────────┐  ┌─────────────┐
│  Buffer 0   │  │  Buffer 1   │
│  (current)  │  │  (previous) │
└──────┬──────┘  └──────┬──────┘
       │                │
       └────── blend ───┘
              motionBlur
                  ↓
            DMA Buffer
```

**Trick**: Blend current and previous frames during update for smooth trails:

```cpp
Color pixel = cube[cubeBuffer][x][y][z].blend(
  motionBlur,
  cube[1 - cubeBuffer][x][y][z]
);
```

**Memory**: 2 × 16×16×16×3 bytes = 24 KB

---

## Animation State Machine

Each animation cycles through states:

```
init()
  ↓
STARTING ──timer─→ RUNNING ──timer─→ ENDING ──timer─→ INACTIVE
 (fade in)         (main)            (fade out)
```

**Brightness scaling**:
- STARTING: `brightness = 255 * ratio()`
- RUNNING: `brightness = 255`
- ENDING: `brightness = 255 * (1 - ratio())`

All animations inherit this lifecycle from base `Animation` class.

---

## Coordinate Boundary Optimization

Instead of 6 comparisons:
```cpp
if (x >= 0 && x < 16 && y >= 0 && y < 16 && z >= 0 && z < 16)
```

Single bitwise operation:
```cpp
if (((x | y | z) & 0xFFF0) == 0)
```

**How it works**:
```
Valid: [0, 15] = 0b0000xxxx  (4 bits)
Invalid: ≥16   = 0b0001xxxx+ (bit 4 set)

x | y | z → If any coordinate ≥16, bits 4+ are set
& 0xFFF0  → Mask to check bits 4+
== 0      → All coords in range
```

See [Graphics.h:22-26](Mega-Cube/Software/LED Display/src/core/Graphics.h#L22-L26)

---

## PLL Configuration

Critical for LED timing - video PLL must generate exactly 711.1 MHz:

```cpp
// Formula: freq = 24 MHz × (loopDiv + num/denom) / postDiv
CCM_ANALOG_PLL_VIDEO_DIV_SELECT(29);      // ×29
CCM_ANALOG_PLL_VIDEO_NUM = 1;
CCM_ANALOG_PLL_VIDEO_DENOM = 3;           // +1/3
CCM_ANALOG_PLL_VIDEO_POST_DIV_SELECT(0);  // ÷4

// Result: 24 × (29 + 1/3) / 4 = 24 × 29.333 / 4 = 711.1 MHz
```

**Why 711.1 MHz**: Matches PL9823-F8 variant clock requirements for proper bit timing.

See [Display.cpp:147-165](Mega-Cube/Software/LED Display/src/core/Display.cpp#L147-L165)

---

## Power Limiting Algorithm

Software current limiter prevents PSU overload:

```cpp
// Calculate total current (60mA per full-white LED)
for each LED:
  current += (r + g + b) * 60 / 765

// Scale if over budget
if (current > max_milliamps) {
  scale = max_milliamps / current
  for each LED:
    LED = LED * scale
}
```

**Budget**: 18000 mA default (18A @ 5V = 90W)

See [Display.cpp:290-315](Mega-Cube/Software/LED Display/src/core/Display.cpp#L290-L315)

---

## ESP8266 Communication

High-speed UART with DMA buffers:

```cpp
Serial1.begin(460800);
Serial1.addMemoryForRead(uart_rx, sizeof(uart_rx));   // DMA RX
Serial1.addMemoryForWrite(uart_tx, sizeof(uart_tx));  // DMA TX
```

**Protocol**: JSON-RPC over UART
```json
{"method": "set_animation", "params": {"index": 3}}
```

**Why DMA**: Non-blocking - WiFi commands processed without interrupting LED updates.

---

## LVGL Integration

LCD uses LVGL 8.3.1 with differential updates:

```cpp
#define LVGL_BUFFER_SIZE (320 * 240 / 10)  // 1/10 screen buffering

lv_disp_draw_buf_init(&draw_buf, buf1, buf2, LVGL_BUFFER_SIZE);
```

**Double buffering**: `buf1` and `buf2` alternate for flicker-free rendering.

**Update strategy**: Only dirty regions redrawn, not full screen.

---

## Configuration Persistence

ArduinoJson + LittleFS for flash storage:

```cpp
struct Config {
  struct { ... } power;
  struct { ... } network;
  struct {
    uint8_t animation;
    struct { ... } atoms;  // 16 animation configs
    // ...
  } animation;
};

extern Config config;  // Global singleton
```

**Serialization**: JSON ↔ struct via ArduinoJson library, stored in `config.json`.

---

## Timing System

Frame-rate independent animations via delta time:

```cpp
class Timer {
  float dt() const {
    return (micros() - time_prev) / 1000000.0;  // Seconds
  }

  float ratio() {
    return constrain((micros() - time_start) / (float)time_alarm, 0.0, 1.0);
  }
};
```

**Usage**:
```cpp
position += velocity * dt;  // Moves same distance regardless of frame rate
```

**Precision**: Microsecond-level via Arduino `micros()`.

---

## Critical Design Patterns

### HAL (Hardware Abstraction Layer)

Graphics code doesn't touch FlexIO/DMA directly:

```cpp
voxel(x, y, z, Color(255, 0, 0));  // Simple API
Display::update();                  // Handles all hardware complexity
```

### Template Method

Base `Animation` class defines lifecycle, derived classes implement `draw()`:

```cpp
class Fireworks : public Animation {
  void init() override { /* custom setup */ }
  void draw(float dt) override { /* custom rendering */ }
};
```

### Singleton

Global config accessible everywhere:

```cpp
config.animation.fireworks.explosion_speed
```

Avoids parameter passing through deep call stacks.

---

## Key Architectural Files

- [main.cpp:29](Mega-Cube/Software/LED Display/src/main.cpp#L29) - Entry point & main loop
- [Display.cpp:97](Mega-Cube/Software/LED Display/src/core/Display.cpp#L97) - LED hardware driver
- [Animation.cpp:98](Mega-Cube/Software/LED Display/src/core/Animation.cpp#L98) - Animation sequencer
- [Config.h:12](Mega-Cube/Software/LED Display/src/core/Config.h#L12) - Configuration structure
- [Graphics.h:13](Mega-Cube/Software/LED Display/src/core/Graphics.h#L13) - Rendering primitives
