#pragma once

#include <cstdint>

// Forward declare GLFW types
struct GLFWwindow;

class Renderer {
public:
    static bool init(int width = 1280, int height = 720);
    static void shutdown();
    static bool shouldClose();
    static void beginFrame();
    static void endFrame();

    // Render the cube buffer
    static void renderCube(uint8_t cube[16][16][16][3], float brightness = 1.0f);

    // Camera control
    static void handleInput();

    // Get delta time
    static float getDeltaTime();

    // Keyboard - returns true only on key press (not held)
    static bool wasKeyPressed(int key);

    // Access window for direct GLFW calls if needed
    static GLFWwindow* getWindow() { return window; }

private:
    static GLFWwindow* window;
    static bool keyStates[512];
    static float cameraAngleX;
    static float cameraAngleY;
    static float cameraDistance;
    static float lastTime;
    static float deltaTime;
    static bool mousePressed;
    static double lastMouseX, lastMouseY;

    static void drawVoxel(float x, float y, float z, float r, float g, float b, float size);
    static void setupCamera();
    static void mouseCallback(GLFWwindow* window, int button, int action, int mods);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
};
