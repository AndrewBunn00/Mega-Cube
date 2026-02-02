#include "Renderer.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#include <cmath>
#include <cstdio>

// Static member definitions
GLFWwindow* Renderer::window = nullptr;
float Renderer::cameraAngleX = 30.0f;
float Renderer::cameraAngleY = 45.0f;
float Renderer::cameraDistance = 40.0f;
float Renderer::lastTime = 0.0f;
float Renderer::deltaTime = 0.0f;
bool Renderer::mousePressed = false;
double Renderer::lastMouseX = 0.0;
double Renderer::lastMouseY = 0.0;
bool Renderer::keyStates[512] = {false};

bool Renderer::init(int width, int height) {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return false;
    }

    window = glfwCreateWindow(width, height, "MEGA CUBE Simulator", nullptr, nullptr);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // VSync

    // Set callbacks
    glfwSetMouseButtonCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);

    // OpenGL setup
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    lastTime = (float)glfwGetTime();

    printf("MEGA CUBE Simulator\n");
    printf("Controls:\n");
    printf("  Left mouse drag: Rotate view\n");
    printf("  Scroll wheel: Zoom in/out\n");
    printf("  ESC: Quit\n\n");

    return true;
}

void Renderer::shutdown() {
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
}

bool Renderer::shouldClose() {
    return glfwWindowShouldClose(window) || glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;
}

void Renderer::beginFrame() {
    // Calculate delta time
    float currentTime = (float)glfwGetTime();
    deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    // Handle input
    handleInput();

    // Get window size
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // Clear
    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Setup camera
    setupCamera();
}

void Renderer::endFrame() {
    glfwSwapBuffers(window);
    glfwPollEvents();
}

float Renderer::getDeltaTime() {
    return deltaTime;
}

bool Renderer::wasKeyPressed(int key) {
    if (key < 0 || key >= 512) return false;

    bool currentState = glfwGetKey(window, key) == GLFW_PRESS;
    bool wasPressed = currentState && !keyStates[key];
    keyStates[key] = currentState;
    return wasPressed;
}

void Renderer::handleInput() {
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        if (mousePressed) {
            float dx = (float)(mouseX - lastMouseX);
            float dy = (float)(mouseY - lastMouseY);
            cameraAngleY += dx * 0.5f;
            cameraAngleX += dy * 0.5f;

            // Clamp vertical angle
            if (cameraAngleX > 89.0f) cameraAngleX = 89.0f;
            if (cameraAngleX < -89.0f) cameraAngleX = -89.0f;
        }

        lastMouseX = mouseX;
        lastMouseY = mouseY;
        mousePressed = true;
    } else {
        mousePressed = false;
    }
}

void Renderer::mouseCallback(GLFWwindow* window, int button, int action, int mods) {
    (void)window; (void)button; (void)action; (void)mods;
}

void Renderer::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    (void)window; (void)xoffset;
    cameraDistance -= (float)yoffset * 2.0f;
    if (cameraDistance < 15.0f) cameraDistance = 15.0f;
    if (cameraDistance > 100.0f) cameraDistance = 100.0f;
}

void Renderer::setupCamera() {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    float aspect = (float)width / (float)height;

    // Projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float fov = 45.0f;
    float zNear = 0.1f;
    float zFar = 200.0f;
    float top = zNear * tanf(fov * 3.14159f / 360.0f);
    float bottom = -top;
    float right = top * aspect;
    float left = -right;
    glFrustum(left, right, bottom, top, zNear, zFar);

    // View matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Camera position based on angles
    float radX = cameraAngleX * 3.14159f / 180.0f;
    float radY = cameraAngleY * 3.14159f / 180.0f;

    float camX = cameraDistance * cosf(radX) * sinf(radY);
    float camY = cameraDistance * sinf(radX);
    float camZ = cameraDistance * cosf(radX) * cosf(radY);

    // Simple lookat
    float upX = 0, upY = 1, upZ = 0;
    float fx = -camX, fy = -camY, fz = -camZ;
    float len = sqrtf(fx*fx + fy*fy + fz*fz);
    fx /= len; fy /= len; fz /= len;

    float sx = fy * upZ - fz * upY;
    float sy = fz * upX - fx * upZ;
    float sz = fx * upY - fy * upX;
    len = sqrtf(sx*sx + sy*sy + sz*sz);
    sx /= len; sy /= len; sz /= len;

    float ux = sy * fz - sz * fy;
    float uy = sz * fx - sx * fz;
    float uz = sx * fy - sy * fx;

    float m[16] = {
        sx, ux, -fx, 0,
        sy, uy, -fy, 0,
        sz, uz, -fz, 0,
        0, 0, 0, 1
    };
    glMultMatrixf(m);
    glTranslatef(-camX, -camY, -camZ);
}

void Renderer::renderCube(uint8_t cube[16][16][16][3], float brightness) {
    const float voxelSize = 0.4f;  // Size of each LED
    const float spacing = 1.0f;    // Spacing between LEDs
    const float offset = 7.5f;     // Center offset

    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
            for (int z = 0; z < 16; z++) {
                float r = cube[x][y][z][0] / 255.0f * brightness;
                float g = cube[x][y][z][1] / 255.0f * brightness;
                float b = cube[x][y][z][2] / 255.0f * brightness;

                // Skip very dim voxels for performance
                if (r < 0.01f && g < 0.01f && b < 0.01f) continue;

                float px = (x - offset) * spacing;
                float py = (y - offset) * spacing;
                float pz = (z - offset) * spacing;

                drawVoxel(px, py, pz, r, g, b, voxelSize);
            }
        }
    }

    // Draw wireframe outline of the cube
    glColor4f(0.3f, 0.3f, 0.3f, 0.5f);
    glBegin(GL_LINES);
    float s = 8.0f;
    // Bottom face
    glVertex3f(-s, -s, -s); glVertex3f( s, -s, -s);
    glVertex3f( s, -s, -s); glVertex3f( s, -s,  s);
    glVertex3f( s, -s,  s); glVertex3f(-s, -s,  s);
    glVertex3f(-s, -s,  s); glVertex3f(-s, -s, -s);
    // Top face
    glVertex3f(-s,  s, -s); glVertex3f( s,  s, -s);
    glVertex3f( s,  s, -s); glVertex3f( s,  s,  s);
    glVertex3f( s,  s,  s); glVertex3f(-s,  s,  s);
    glVertex3f(-s,  s,  s); glVertex3f(-s,  s, -s);
    // Vertical edges
    glVertex3f(-s, -s, -s); glVertex3f(-s,  s, -s);
    glVertex3f( s, -s, -s); glVertex3f( s,  s, -s);
    glVertex3f( s, -s,  s); glVertex3f( s,  s,  s);
    glVertex3f(-s, -s,  s); glVertex3f(-s,  s,  s);
    glEnd();
}

void Renderer::drawVoxel(float x, float y, float z, float r, float g, float b, float size) {
    // Draw as a simple point/sphere for performance
    // Could be upgraded to actual cubes if needed

    glPointSize(size * 15.0f);  // Scale point size
    glBegin(GL_POINTS);
    glColor3f(r, g, b);
    glVertex3f(x, y, z);
    glEnd();
}
