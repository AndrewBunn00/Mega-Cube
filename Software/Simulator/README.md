# MEGA CUBE Simulator

Test LED cube animations without flashing hardware. Same C++ animation code runs on both simulator and real cube.

## Features

- Real-time 3D visualization of 16×16×16 cube
- Mouse controls for rotating view
- Shares animation code with LED Display firmware
- 5 built-in demo animations

## Controls

- **Left mouse drag**: Rotate view
- **Scroll wheel**: Zoom in/out
- **ESC**: Quit

## Building

### Requirements

- CMake 3.16+
- C++17 compiler (MSVC, GCC, or Clang)
- OpenGL (usually pre-installed)

### Windows (Visual Studio)

```bash
cd "Mega-Cube/Software/Simulator"
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

Then run:
```bash
Release\simulator.exe
```

### Windows (MinGW/MSYS2)

```bash
cd "Mega-Cube/Software/Simulator"
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
./simulator.exe
```

### Linux/macOS

```bash
cd "Mega-Cube/Software/Simulator"
mkdir build && cd build
cmake ..
make -j$(nproc)
./simulator
```

## Demo Animations

The simulator includes 5 demo animations that demonstrate the cube's capabilities:

1. **Plasma** - 4D Perlin noise volumetric effect
2. **Rotating Cube** - Wireframe cube with quaternion rotation
3. **Atoms** - 9 electrons orbiting on dynamic axes
4. **Sinus Wave** - 3D wave surface
5. **Starfield** - Perspective projection star field

## Adding Custom Animations

To test your own animations:

1. Create a class inheriting from `DemoAnimation`:

```cpp
class MyAnimation : public DemoAnimation {
public:
    void init() override {
        // Setup
    }

    void update(float dt) override {
        // Draw frame using Graphics:: functions
        Graphics::voxel(x, y, z, Color(r, g, b));
        Graphics::radiate(cx, cy, cz, color, radius);
        Graphics::line(x1, y1, z1, x2, y2, z2, color);
    }

    const char* name() override { return "My Animation"; }
};
```

2. Add it to the `demos[]` array in `main.cpp`

3. Rebuild

## Using Real Animation Code

The simulator shares math libraries with the real firmware:
- `Math3D.h/.cpp` - Vector3, Quaternion
- `Noise.h/.cpp` - Perlin noise
- `Color.h/.cpp` - Color handling
- `Timer.h/.cpp` - Timing utilities
- `Particle.h/.cpp` - Particle physics

To use an animation from the real firmware:

1. Include the animation header
2. Ensure config structures are defined in SimDisplay.h
3. Call the animation's init/draw methods

## Architecture

```
Simulator/
├── src/
│   ├── main.cpp       # Entry point and demo animations
│   ├── Renderer.cpp   # OpenGL 3D rendering
│   ├── SimDisplay.cpp # Simulator Display (replaces hardware driver)
│   └── Arduino.h      # Arduino compatibility shim
└── CMakeLists.txt     # Build configuration
```

The key abstraction is `SimDisplay` which provides the same interface as the real `Display` class but renders to an OpenGL window instead of driving LEDs.

## Troubleshooting

### Build fails: "Could not find OpenGL"

Install OpenGL development libraries:
- Windows: Usually included with graphics drivers
- Linux: `sudo apt install libgl1-mesa-dev`
- macOS: Included with Xcode

### GLFW fetch fails

CMake will auto-download GLFW. Ensure you have internet access or install GLFW manually:
- Windows: `vcpkg install glfw3`
- Linux: `sudo apt install libglfw3-dev`
- macOS: `brew install glfw`

### Animation looks different from real cube

The simulator uses point rendering for performance. Real LEDs have different color characteristics and blending. Use the simulator for logic testing, not exact color matching.
