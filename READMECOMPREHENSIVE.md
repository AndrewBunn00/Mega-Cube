# MEGA CUBE - 16×16×16 LED Cube

Real-time 3D LED visualization system with 4096 addressable RGB LEDs controlled by Teensy 4.0.

## Features

- **16 Pre-programmed Animations** - Fireworks, plasma, starfield, Game of Life, and more
- **30 FPS Real-Time Rendering** - DMA-driven with motion blur support
- **Advanced Math** - Quaternion rotations, Perlin noise, particle physics
- **Network Control** - WiFi via ESP8266 with JSON-RPC interface
- **LCD Touchscreen** - Local control and configuration
- **Power Management** - Software current limiting up to 18A

## Hardware

- **Teensy 4.0** - 600 MHz ARM Cortex-M7 controller
- **4096 PL9823-F8 LEDs** - Addressable RGB (GRB order)
- **ILI9341 LCD** - 320×240 touchscreen
- **ESP8266** - WiFi module
- **5V @ 18A PSU** - Switching power supply

## Quick Start

### Build Firmware

```bash
cd "Mega-Cube/Software/LED Display"
platformio run --target upload
```

### Configuration

Edit [`config.json`](Mega-Cube/Software/LED Display/data/config.json) or use LCD/WiFi interface:

```json
{
  "power": {
    "max_milliamps": 18000,
    "brightness": 1.0
  },
  "network": {
    "ssid": "YourWiFi",
    "password": "password"
  },
  "animation": {
    "animation": 0,
    "play_one": false
  }
}
```

## Project Structure

```
Mega-Cube/
├── Software/
│   ├── LED Display/          # Main Teensy firmware ⭐
│   │   ├── src/
│   │   │   ├── core/         # Display, config, graphics
│   │   │   ├── space/        # 16 animations
│   │   │   ├── power/        # Math, color, particles, noise
│   │   │   └── main.cpp      # Entry point
│   │   └── platformio.ini
│   ├── WIFI Module/          # ESP8266 firmware
│   └── WIO Terminal/         # Remote control app
├── Electronics/              # KiCAD schematics & PCBs
├── Construction/             # Assembly guides
└── Documents/                # Technical docs
```

## Documentation

- **[MATHEMATICS.md](MATHEMATICS.md)** - Math concepts (quaternions, noise, vectors)
- **[ARCHITECTURE.md](ARCHITECTURE.md)** - Complex implementation details
- **[ANIMATIONS.md](ANIMATIONS.md)** - Animation system and creating custom effects
- **[HARDWARE.md](HARDWARE.md)** - Pin connections, power, troubleshooting

## Key Technical Details

### DMA Ping-Pong LED Updates

```
Channel 0: ████████░░░░░░░░  (DMA transferring to LEDs)
Channel 1: ░░░░░░░░████████  (CPU filling buffer)
```

Continuous 30 FPS output with no frame drops.

### Double Buffering with Motion Blur

```cpp
Color pixel = cube[current][x][y][z].blend(
  motionBlur,
  cube[previous][x][y][z]
);
```

Smooth trails by blending current and previous frames.

### Critical PLL Setup

```cpp
// 24 MHz × (29 + 1/3) / 4 = 711.1 MHz (required for PL9823)
CCM_ANALOG_PLL_VIDEO_DIV_SELECT(29);
CCM_ANALOG_PLL_VIDEO_NUM = 1;
CCM_ANALOG_PLL_VIDEO_DENOM = 3;
```

Wrong frequency → flickering LEDs.

### GRB Color Order

PL9823 LEDs use **GRB** (not RGB). Code handles conversion automatically.

## Animations

1. **Atoms** - 9 electrons on dynamic rotation axes
2. **Sinus** - 3D wave surface
3. **Starfield** - Perspective projection starfield
4. **Fireworks** - Particle physics explosions
5. **Twinkels** - Gaussian fade twinkling
6. **Helix** - DNA double helix
7. **Arrows** - Rotating arrow spirals
8. **Plasma** - 4D Perlin noise volumetric
9. **Mario** - Sprite rendering
10. **Life** - 3D Conway's Game of Life
11. **Pong** - 3D Pong game
12. **Spectrum** - Audio FFT visualization
13. **Scroller** - Cylindrical text display
14. **Accelerometer** - IMU-driven particles
15. **Cube** - Wireframe cube rotation
16. **Countdown** - Numeric countdown

See [ANIMATIONS.md](ANIMATIONS.md) for details and how to create custom animations.

## Math Highlights

### Quaternions (Gimbal-Lock Free)

```cpp
// Rotate by θ around axis k
q.w = cos(θ/2)
q.v = sin(θ/2) * k

// Apply rotation
v' = q·v·q⁻¹
```

### Perlin Noise (Organic Patterns)

```cpp
// 4D noise for volumetric effects
value = noise.noise4(x, y, z, time);
```

### Particle Physics

```cpp
velocity += gravity * dt;
position += velocity * dt;
```

See [MATHEMATICS.md](MATHEMATICS.md) for detailed explanations with ASCII diagrams.

## Performance

| Metric | Value |
|--------|-------|
| Frame Rate | ~30 FPS |
| LEDs | 4096 |
| Memory | 24 KB cube buffers + 25 KB LVGL |
| Power | 18 A @ 5V (90W) typical |
| Timing Precision | Microsecond-level |

## Creating Custom Animations

Inherit from `Animation` base class:

```cpp
class MyAnimation : public Animation {
  void init() override { /* setup */ }
  void draw(float dt) override { /* render frame */ }
};
```

Add config, register in `Animation::get_item()`, and compile.

See [ANIMATIONS.md](ANIMATIONS.md) for full template.

## Network Control

ESP8266 provides JSON-RPC interface:

```json
{"method": "set_animation", "params": {"index": 3}}
```

UART @ 460800 baud with DMA buffers for non-blocking communication.

## Power Management

Software current limiter scales brightness to stay under 18A budget:

```cpp
if (total_current > max_milliamps) {
  scale = max_milliamps / total_current;
  // Scale all LED values
}
```

## Building and Flashing

**Requirements**: [PlatformIO](https://platformio.org/)

```bash
cd "Mega-Cube/Software/LED Display"
platformio run --target upload
```

**Dependencies** (auto-installed):
- ArduinoJson 6.19.2
- LVGL 8.3.1

## Troubleshooting

- **No LEDs**: Check 5V power, verify PLL 711.1 MHz
- **Wrong colors**: Verify GRB order (should be handled)
- **Flickering**: Add power injection points, check voltage
- **LCD blank**: Check 3.3V supply
- **WiFi fails**: Verify 460800 baud, check GPIO0 state

See [HARDWARE.md](HARDWARE.md) for detailed troubleshooting.

## File Reference

**Core Files**:
- [main.cpp:29](Mega-Cube/Software/LED Display/src/main.cpp#L29) - Entry point
- [Display.cpp:97](Mega-Cube/Software/LED Display/src/core/Display.cpp#L97) - LED driver
- [Animation.cpp:98](Mega-Cube/Software/LED Display/src/core/Animation.cpp#L98) - Animation sequencer
- [Math3D.cpp](Mega-Cube/Software/LED Display/src/power/Math3D.cpp) - Vectors & quaternions
- [Noise.cpp](Mega-Cube/Software/LED Display/src/power/Noise.cpp) - Perlin noise
- [Graphics.h](Mega-Cube/Software/LED Display/src/core/Graphics.h) - Rendering primitives

## License

[Specify license]

## Credits

Hardware and software design by [Author/Team]
