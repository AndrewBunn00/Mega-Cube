/*
 * MEGA CUBE Simulator
 *
 * Test LED cube animations without flashing hardware.
 * Direct ports of the real cube animations.
 *
 * Controls:
 *   Left mouse drag: Rotate view
 *   Scroll wheel: Zoom in/out
 *   Space: Next animation
 *   R: Reset animation
 *   ESC: Quit
 */

#include "Renderer.h"
#include "SimDisplay.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <algorithm>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

// Simple animation base class for demos
class DemoAnimation {
public:
    virtual ~DemoAnimation() = default;
    virtual void init() = 0;
    virtual void update(float dt) = 0;
    virtual const char* name() = 0;
};

//=============================================================================
// Demo 1: Plasma (Direct port from real cube)
//=============================================================================
class PlasmaDemo : public DemoAnimation {
    float speed_offset = 0;
    float speed_offset_speed = 0.5f;
    float scale_p = 0.15f;
    float speed_x, speed_y, speed_z, speed_w;

    float noise_x, noise_y, noise_z, noise_w;
    uint8_t noise_map[16][16][16];
    uint16_t hue16 = 0;

public:
    void init() override {
        speed_offset = 0;
        noise_x = noise.nextRandom(0, 255);
        noise_y = noise.nextRandom(0, 255);
        noise_z = noise.nextRandom(0, 255);
        noise_w = noise.nextRandom(0, 255);
        hue16 = 0;
    }

    const char* name() override { return "Plasma (4D Noise)"; }

    void update(float dt) override {
        // Update noise parameters (from real Plasma.h)
        speed_offset += dt * speed_offset_speed;
        speed_x = 2 * (noise.noise1(speed_offset + 0) - 0.5f);
        speed_y = 2 * (noise.noise1(speed_offset + 50) - 0.5f);
        speed_z = 2 * (noise.noise1(speed_offset + 100) - 0.5f);
        speed_w = 2 * (noise.noise1(speed_offset + 150) - 0.5f);
        scale_p = 0.15f + (noise.noise1(speed_offset + 200) / 6.6f);

        noise_x += speed_x * dt;
        noise_y += speed_y * dt;
        noise_z += speed_z * dt;
        noise_w += speed_w * dt;

        hue16 += (uint16_t)(dt * 50 * 255);

        // Generate noise map
        for (int x = 0; x < 16; x++) {
            float xoffset = noise_x + scale_p * x;
            for (int y = 0; y < 16; y++) {
                float yoffset = noise_y + scale_p * y;
                for (int z = 0; z < 16; z++) {
                    float zoffset = noise_z + scale_p * z;
                    noise_map[x][y][z] = (uint8_t)(noise.noise4(xoffset, yoffset, zoffset, noise_w) * 255);
                }
            }
        }

        // Draw using LavaPalette (same as real cube)
        for (int x = 0; x < 16; x++) {
            for (int y = 0; y < 16; y++) {
                for (int z = 0; z < 16; z++) {
                    uint8_t index = noise_map[x][y][z];
                    Color c = Color((hue16 >> 8) + index, LavaPalette);
                    c.scale(noise_map[y][x][z]);
                    voxel((uint8_t)x, (uint8_t)y, (uint8_t)z, c);
                }
            }
        }
    }
};

//=============================================================================
// Demo 2: Cube (Direct port from real cube)
//=============================================================================
class CubeDemo : public DemoAnimation {
    float angle = 0;
    uint16_t hue16 = 0;

    // Settings
    float angle_speed = 1.0f;
    float hue_speed = 50.0f;
    float radius = 6.0f;
    float distance = 1.5f;

    Vector3 va = Vector3(-1, -1, -1);
    Vector3 vb = Vector3(+1, -1, -1);
    Vector3 vc = Vector3(-1, +1, -1);
    Vector3 vd = Vector3(+1, +1, -1);
    Vector3 ve = Vector3(-1, -1, +1);
    Vector3 vf = Vector3(+1, -1, +1);
    Vector3 vg = Vector3(-1, +1, +1);
    Vector3 vh = Vector3(+1, +1, +1);
    Vector3 polygon[12][2] = {
        {va, vb}, {vc, vd}, {va, vc}, {vb, vd}, {ve, vf}, {vg, vh},
        {ve, vg}, {vf, vh}, {va, ve}, {vb, vf}, {vc, vg}, {vd, vh}
    };

public:
    void init() override {
        angle = 0;
        hue16 = 0;
    }

    const char* name() override { return "Cube (Real Port)"; }

    void update(float dt) override {
        angle += dt * angle_speed;
        hue16 += (uint16_t)(dt * hue_speed * 255);
        uint8_t pixnr = 0;

        // Cycle through rotation axes (from real Cube.h)
        Quaternion q = Quaternion(angle, Vector3(0, 0, 1));
        if (angle > 6 * 360) angle -= 6 * 360;
        if (angle > 4 * 360) {
            q = Quaternion(angle, Vector3(0, 1, 0));
        } else if (angle > 2 * 360) {
            q = Quaternion(angle, Vector3(1, 1, 1));
        }

        for (uint16_t i = 0; i < 12; i++) {
            Vector3 v1 = q.rotate(polygon[i][0] * radius);
            Vector3 v2 = q.rotate(polygon[i][1] * radius);
            Vector3 n = v1 - v2;
            float steps = 1 + std::max(std::abs(n.z), std::max(std::abs(n.x), std::abs(n.y)));
            Vector3 inc = n / steps;

            for (uint8_t j = 0; j <= (uint8_t)steps; j++) {
                Color c = Color(pixnr += 6, RainbowGradientPalette);
                radiate(v1 - (inc * j), c, distance);
            }
        }
    }
};

//=============================================================================
// Demo 3: Atoms (Direct port from real cube)
//=============================================================================
class AtomsDemo : public DemoAnimation {
    float angle = 0;
    uint16_t hue16 = 0;

    float angle_speed = 1.0f;
    float hue_speed = 50.0f;
    float radius = 6.5f;
    float distance = 3.0f;

public:
    void init() override {
        angle = 0;
        hue16 = 0;
    }

    const char* name() override { return "Atoms (Real Port)"; }

    void update(float dt) override {
        angle += dt * angle_speed;
        hue16 += (uint16_t)(dt * hue_speed * 255);

        float a = angle * 1.0f;
        float t = angle * 0.1f;

        Quaternion axes[] = {
            Quaternion(t, Vector3(+sinf(a / 95), +sinf(a / 75), -sinf(a / 95))),
            Quaternion(t, Vector3(+sinf(a / 90), -sinf(a / 85), -sinf(a / 95))),
            Quaternion(t, Vector3(-sinf(a / 94), +sinf(a / 80), -sinf(a / 75))),
            Quaternion(t, Vector3(+sinf(a / 90), +sinf(a / 70), -sinf(a / 90))),
            Quaternion(t, Vector3(+sinf(a / 80), -sinf(a / 70), -sinf(a / 99))),
            Quaternion(t, Vector3(-sinf(a / 99), +sinf(a / 90), -sinf(a / 80))),
            Quaternion(t, Vector3(-sinf(a / 90), -sinf(a / 90), +sinf(a / 99))),
            Quaternion(t, Vector3(-sinf(a / 70), -sinf(a / 80), -sinf(a / 90))),
            Quaternion(t, Vector3(-sinf(a / 99), +sinf(a / 70), +sinf(a / 80)))
        };

        Vector3 atoms[] = {
            Vector3(1, 0, 0),
            Vector3(0, 1, 0),
            Vector3(0, 0, 1),
            Vector3(-1, 0, 0),
            Vector3(0, -1, 0),
            Vector3(0, 0, -1),
            Vector3(1, 0, 1).normalize(),
            Vector3(1, 1, 0).normalize(),
            Vector3(0, 1, 1).normalize()
        };

        for (uint8_t i = 0; i < 9; i++) {
            Vector3 v = axes[i].rotate(atoms[i]) * radius;
            Color c = Color((hue16 >> 8) + (i * 8), RainbowGradientPalette);
            radiate5(v, c, distance);
        }
    }
};

//=============================================================================
// Demo 4: Sinus Wave (Direct port from real cube)
//=============================================================================
class SinusDemo : public DemoAnimation {
    float phase = 0;
    uint16_t hue16 = 0;

    float x_min = -2.0f, x_max = 2.0f;
    float z_min = -2.0f, z_max = 2.0f;
    float radius = 7.5f;
    float resolution = 32.0f;
    float phase_speed = 1.0f;
    float hue_speed = 50.0f;

public:
    void init() override {
        phase = 0;
        hue16 = 0;
    }

    const char* name() override { return "Sinus (Real Port)"; }

    void update(float dt) override {
        phase += dt * phase_speed;
        hue16 += (uint16_t)(dt * hue_speed * 255);

        Quaternion q = Quaternion(phase * 10.0f, Vector3(1, 1, 1));

        for (uint16_t x = 0; x <= (uint16_t)resolution; x++) {
            float xprime = mapf((float)x, 0, resolution, x_min, x_max);
            for (uint16_t z = 0; z <= (uint16_t)resolution; z++) {
                float zprime = mapf((float)z, 0, resolution, z_min, z_max);
                float y = sinf(phase + sqrtf(xprime * xprime + zprime * zprime));

                Vector3 point = Vector3(
                    2.0f * (x / resolution) - 1.0f,
                    2.0f * (z / resolution) - 1.0f,
                    y
                );
                point = q.rotate(point) * radius;

                Color c = Color((hue16 >> 8) + (int8_t)(y * 64), RainbowGradientPalette);
                radiate(point, c, 1.0f);
            }
        }
    }
};

//=============================================================================
// Demo 5: Starfield (Direct port from real cube)
//=============================================================================
class StarfieldDemo : public DemoAnimation {
    static const int numStars = 200;
    Vector3 stars[numStars];
    bool initialized = false;

    float phase = 0;
    uint16_t hue16 = 0;
    float phase_speed = 1.0f;
    float hue_speed = 50.0f;
    float body_diagonal = 13.0f;

public:
    void init() override {
        phase = 0;
        hue16 = 0;

        if (!initialized) {
            for (int i = 0; i < numStars; i++) {
                stars[i] = Vector3(
                    noise.nextRandom(-1, 1),
                    noise.nextRandom(-1, 1),
                    noise.nextRandom(-1, 1)
                );
            }
            initialized = true;
        }
    }

    const char* name() override { return "Starfield (Real Port)"; }

    void update(float dt) override {
        phase += dt * phase_speed;
        hue16 += (uint16_t)(dt * hue_speed * 255);

        Quaternion q = Quaternion(25.0f * phase, Vector3(0, 1, 0));

        for (int i = 0; i < numStars; i++) {
            float r = (stars[i] * 3 - Vector3(0, 0, -2.0f)).magnitude();
            stars[i].z += sinf(phase) * 1.75f * dt * r;

            if (stars[i].z > 1) {
                stars[i] = Vector3(noise.nextRandom(-1, 1), noise.nextRandom(-1, 1), -1);
            } else if (stars[i].z < -1) {
                stars[i] = Vector3(noise.nextRandom(-1, 1), noise.nextRandom(-1, 1), 1);
            }

            Color c = Color((hue16 >> 8) + (int8_t)(r * 6), RainbowGradientPalette);
            voxel(q.rotate(stars[i]) * body_diagonal, c);
        }
    }
};

//=============================================================================
// Demo 6: Helix (Direct port from real cube)
//=============================================================================
class HelixDemo : public DemoAnimation {
    float phase = 0;
    float angle = 0;
    uint16_t hue16 = 0;

    float phase_speed = 2.0f;
    float angle_speed = 0.5f;
    float hue_speed = 50.0f;
    float radius = 7.0f;
    float resolution = 32.0f;
    uint8_t thickness = 3;

public:
    void init() override {
        phase = 0;
        angle = 0;
        hue16 = 0;
    }

    const char* name() override { return "Helix (Real Port)"; }

    void update(float dt) override {
        phase += dt * phase_speed;
        angle += dt * angle_speed;
        hue16 += (uint16_t)(dt * hue_speed * 255);

        Quaternion q1 = Quaternion(180, Vector3(0, 1, 0));
        Quaternion q2 = Quaternion(angle, Vector3(1, 0, 0));

        for (uint16_t y = 0; y <= (uint16_t)resolution; y++) {
            float xf = sinf(phase + mapf((float)y, 0, resolution, 0, 2 * PI));
            float zf = cosf(phase + mapf((float)y, 0, resolution, 0, 2 * PI));
            Vector3 p0 = Vector3(xf, 2 * (y / resolution) - 1, zf) * radius;
            Vector3 p1 = q2.rotate(p0);
            Vector3 p2 = (q2 * q1).rotate(p0);

            Color c1 = Color((hue16 >> 8) + y * 2 + 0, RainbowGradientPalette);
            Color c2 = Color((hue16 >> 8) + y * 2 + 128, RainbowGradientPalette);

            radiate(p1, c1, 1.0f + (float)thickness / 20.0f);
            radiate(p2, c2, 1.0f + (float)thickness / 20.0f);
        }
    }
};

//=============================================================================
// Demo 7: Fireworks (Direct port from real cube)
//=============================================================================
class FireworksDemo : public DemoAnimation {
    static const int MAX_DEBRIS = 200;
    float radius = 7.5f;
    uint16_t numDebris = 0;
    Vector3 source, target, velocity, gravity;
    Particle missile;
    Particle debris[MAX_DEBRIS];
    bool exploded = false;

public:
    void init() override {
        fireArrow();
    }

    void fireArrow() {
        source = Vector3(noise.nextGaussian(0.0f, 0.25f), -1.0f,
                         noise.nextGaussian(0.0f, 0.25f));
        target = Vector3(noise.nextGaussian(0.0f, 0.25f),
                         noise.nextGaussian(0.8f, 0.10f),
                         noise.nextGaussian(0.0f, 0.25f));
        float t = noise.nextGaussian(0.60f, 0.20f);
        velocity = (target - source) / t;
        missile.position = source;
        missile.velocity = velocity;
        gravity = Vector3(0, -1.0f, 0);
        exploded = false;
    }

    const char* name() override { return "Fireworks (Real Port)"; }

    void update(float dt) override {
        if (!exploded) {
            Vector3 temp = missile.position;
            missile.move(dt, gravity);

            if ((temp.y > missile.position.y) || (missile.position.y > target.y)) {
                exploded = true;
                numDebris = (uint16_t)((rand() % (MAX_DEBRIS / 2)) + MAX_DEBRIS / 2);
                float pwr = noise.nextRandom(0.50f, 1.00f);
                uint8_t hue = (uint8_t)(rand() % 256);

                for (uint16_t i = 0; i < numDebris; i++) {
                    Vector3 explode = Vector3(
                        noise.nextRandom(-pwr, pwr),
                        noise.nextRandom(-pwr, pwr),
                        noise.nextRandom(-pwr, pwr)
                    );
                    debris[i] = Particle(temp, explode, (uint8_t)(hue + rand() % 64),
                                         1.0f, noise.nextRandom(1.0f, 2.0f));
                }
            } else {
                voxel(missile.position * radius, Color::WHITE);
            }
        }

        if (exploded) {
            uint16_t visible = 0;
            for (uint16_t i = 0; i < numDebris; i++) {
                if (debris[i].position.y > -1.0f)
                    debris[i].move(dt, gravity);
                else
                    debris[i].position.y = -1.0f;

                if (debris[i].brightness > 0) {
                    visible++;
                    debris[i].brightness -= dt * (1 / debris[i].seconds);
                } else {
                    debris[i].brightness = 0;
                }

                Color c = Color(debris[i].hue, RainbowGradientPalette);
                if (rand() % 20 == 0) c = Color::WHITE;
                c.scale((uint8_t)(debris[i].brightness * 255));
                voxel_add(debris[i].position * radius, c);
            }

            if (visible == 0) {
                fireArrow();
            }
        }
    }
};

//=============================================================================
// Demo 8: Life (3D Game of Life - Direct port)
//=============================================================================
class LifeDemo : public DemoAnimation {
    uint16_t cells_g1[16][16];
    uint16_t cells_g2[16][16];
    enum class rule_t : uint8_t { DIE = 0, LIVE = 1, BIRTH = 2 };
    rule_t rules[27];
    uint32_t hash_list[256];
    uint32_t hash_nr = 0;
    uint16_t living = 0;
    uint8_t sequence = 0;
    float time_phase = 0;
    float time_interval = 0.15f;

public:
    void init() override {
        game_reset();
    }

    const char* name() override { return "Life 3D (Real Port)"; }

    void game_reset() {
        living = hash_nr = 0;
        for (uint8_t x = 0; x < 16; x++)
            for (uint8_t y = 0; y < 16; y++) {
                cells_g1[x][y] = 0;
                cells_g2[x][y] = 0;
            }
        game_rule_reset();
    }

    void game_rule_reset() {
        for (uint8_t i = 0; i < 27; i++) rules[i] = rule_t::DIE;
    }

    void game_randomize(uint16_t amount, float rad) {
        for (int16_t x = 0; x < 16; x++)
            for (int16_t y = 0; y < 16; y++)
                cells_g2[x][y] = 0;

        for (uint16_t i = 0; i < amount; i++) {
            float r = noise.nextRandom(0, rad);
            float theta = noise.nextRandom(0, 2 * PI);
            float psi = noise.nextRandom(0, 2 * PI);
            uint16_t x = (uint16_t)roundf(7.5f + r * sinf(psi) * cosf(theta));
            uint16_t y = (uint16_t)roundf(7.5f + r * sinf(psi) * sinf(theta));
            uint16_t z = (uint16_t)roundf(7.5f + r * cosf(psi));
            if (x < 16 && y < 16 && z < 16)
                cells_g2[x][y] |= (1 << z);
        }
        living = amount;
    }

    uint16_t count_neighbours(int16_t x_, int16_t y_, int16_t z_) {
        uint16_t neighbours = 0;
        for (int16_t x = x_ - 1; x <= x_ + 1; x++)
            for (int16_t y = y_ - 1; y <= y_ + 1; y++) {
                uint16_t cells = cells_g1[x & 0xF][y & 0xF];
                for (int16_t z = z_ - 1; z <= z_ + 1; z++) {
                    uint16_t cell = cells & (1 << (z & 0xF));
                    if (!(x == x_ && y == y_ && z == z_) && cell)
                        neighbours++;
                }
            }
        return neighbours;
    }

    uint32_t game_next_generation() {
        uint16_t new_living = 0;
        uint32_t hash = 0;
        memcpy(cells_g1, cells_g2, sizeof(cells_g1));
        for (int16_t x = 0; x < 16; x++) {
            for (int16_t y = 0; y < 16; y++) {
                uint16_t cells = cells_g1[x][y];
                for (int16_t z = 0; z < 16; z++) {
                    uint16_t count = count_neighbours(x, y, z);
                    hash += count * (x * 3 + y * 5 + z * 7);
                    if (rules[count] == rule_t::DIE)
                        cells &= ~(1 << z);
                    else if (rules[count] == rule_t::BIRTH)
                        cells |= (1 << z);
                    if (cells & (1 << z)) new_living++;
                }
                cells_g2[x][y] = cells;
            }
        }
        living = new_living;
        return hash;
    }

    void game_progress() {
        uint32_t hash = game_next_generation();
        if (living == 0) {
            game_reset();
            switch (sequence++) {
                case 0:  // Life 4555
                    rules[4] = rule_t::LIVE;
                    rules[5] = rule_t::BIRTH;
                    game_randomize((uint16_t)(200 + rand() % 200), noise.nextRandom(5.0f, 7.0f));
                    break;
                case 1:  // Life 5766
                    rules[5] = rule_t::LIVE;
                    rules[7] = rule_t::LIVE;
                    rules[6] = rule_t::BIRTH;
                    game_randomize((uint16_t)(200 + rand() % 200), noise.nextRandom(5.0f, 7.0f));
                    break;
                case 2:  // Life 5655
                    rules[5] = rule_t::BIRTH;
                    rules[6] = rule_t::LIVE;
                    game_randomize((uint16_t)(200 + rand() % 200), noise.nextRandom(5.0f, 7.0f));
                    break;
                case 3:  // Life 5855
                default:
                    sequence = 0;
                    rules[5] = rule_t::BIRTH;
                    rules[6] = rule_t::LIVE;
                    rules[7] = rule_t::LIVE;
                    rules[8] = rule_t::LIVE;
                    game_randomize(25, 3.0f);
                    break;
            }
        } else {
            uint8_t matches = 0;
            for (uint8_t i = 0; i < hash_nr && i <= 255; i++)
                if (hash_list[i] == hash) matches++;
            if (matches >= 6 || living >= 500) {
                game_rule_reset();
                game_next_generation();
            } else {
                hash_list[hash_nr++ & 0xFF] = hash;
            }
        }
    }

    void update(float dt) override {
        time_phase += dt;
        float scale = 0;
        uint8_t index = 0;

        if (time_phase <= time_interval) {
            scale = 255 * time_phase / time_interval;
        } else if (time_phase <= 2 * time_interval) {
            index = 1;
            scale = 255 * (time_phase - time_interval) / time_interval;
        } else {
            time_phase = 0.0f;
            game_progress();
        }

        Color alive = Color(255, 150, 30);
        Color sparkle = Color(255, 30, 150);
        Color dead = Color::BLACK;
        Color dieing = Color(150, 0, 0);
        Color birth = Color(150, 255, 0);

        Color colors[6] = {
            Color((uint8_t)scale, alive, sparkle), Color((uint8_t)scale, sparkle, alive),
            Color((uint8_t)scale, alive, dieing),  Color((uint8_t)scale, dieing, dead),
            Color((uint8_t)scale, dead, birth),    Color((uint8_t)scale, birth, alive)
        };

        for (int16_t x = 0; x < 16; x++)
            for (int16_t y = 0; y < 16; y++) {
                uint16_t g1 = cells_g1[x][y];
                uint16_t g2 = cells_g2[x][y];
                for (int16_t z = 0; z < 16; z++) {
                    uint16_t i = index;
                    uint16_t mask = (1 << z);
                    if (g1 & g2 & mask) i += 0;
                    else if (g1 & ~g2 & mask) i += 2;
                    else if (~g1 & g2 & mask) i += 4;
                    else continue;
                    voxel((uint8_t)x, (uint8_t)y, (uint8_t)z, colors[i]);
                }
            }
    }
};

//=============================================================================
// Demo 9: Twinkels (Direct port from real cube)
//=============================================================================
class TwinkelsDemo : public DemoAnimation {
    Color colors[16][16][16];
    float duration[16][16][16];
    float timer = 0;
    float interval = 0.01f;
    float fade_in_speed = 0.5f;
    float fade_out_speed = 1.5f;

public:
    void init() override {
        timer = 0;
        for (uint8_t x = 0; x < 16; x++)
            for (uint8_t y = 0; y < 16; y++)
                for (uint8_t z = 0; z < 16; z++) {
                    duration[x][y][z] = 0;
                    colors[x][y][z] = Color::BLACK;
                }
    }

    const char* name() override { return "Twinkels (Real Port)"; }

    void update(float dt) override {
        timer += dt;

        for (uint8_t x = 0; x < 16; x++) {
            for (uint8_t y = 0; y < 16; y++) {
                for (uint8_t z = 0; z < 16; z++) {
                    if (!colors[x][y][z].isBlack()) {
                        if (duration[x][y][z] < fade_in_speed) {
                            float t = duration[x][y][z] / fade_in_speed;
                            voxel(x, y, z, colors[x][y][z].scaled((uint8_t)(255 * t)));
                            duration[x][y][z] += dt;
                        } else if (duration[x][y][z] < (fade_in_speed + fade_out_speed)) {
                            float t = (duration[x][y][z] - fade_in_speed) / fade_out_speed;
                            voxel(x, y, z, colors[x][y][z].scaled((uint8_t)(255 * (1 - t))));
                            duration[x][y][z] += dt;
                        } else {
                            duration[x][y][z] = 0;
                            colors[x][y][z] = Color::BLACK;
                        }
                    }
                }
            }
        }

        // Add new twinkles
        if (timer >= interval) {
            timer = 0;
            uint8_t x = rand() % 16;
            uint8_t y = rand() % 16;
            uint8_t z = rand() % 16;
            if (duration[x][y][z] == 0) {
                colors[x][y][z] = Color((uint8_t)(rand() % 256), 255);  // Random bright color
            }
        }
    }
};

//=============================================================================
// Demo 10: Arrows (Direct port from real cube)
//=============================================================================
class ArrowsDemo : public DemoAnimation {
    float angle = 0;
    uint16_t hue16 = 0;

    float angle_speed = 2.0f;
    float hue_speed = 50.0f;
    float radius = 7.0f;
    float distance = 1.5f;
    uint8_t brightness = 255;

    const uint16_t bitmap[10] = {
        0b0000000110000000,  // 0
        0b0000001111000000,  // 1
        0b0000011111100000,  // 2
        0b0000111111110000,  // 3
        0b0001111111111000,  // 4
        0b0011111111111100,  // 5
        0b0000001111000000,  // 6
        0b0000001111000000,  // 7
        0b0000001111000000,  // 8
        0b0000001111000000,  // 9
    };

public:
    void init() override {
        angle = 0;
        hue16 = 0;
    }

    const char* name() override { return "Arrows (Real Port)"; }

    void update(float dt) override {
        angle += dt * angle_speed;
        hue16 += (uint16_t)(dt * hue_speed * 255);

        float arc = 2 * (180 / PI) * asinf(0.5f / radius);

        // Rotation around arbitrary axes to add chaos
        Quaternion q2 = Quaternion(angle * 0.6f, Vector3(1, 1, 1));
        Quaternion q3 = Quaternion(angle * 0.7f, Vector3(-1, -1, -1));
        Quaternion q4 = Quaternion(angle * 0.8f, Vector3(0, 1, 0));

        for (uint8_t y = 0; y < 10; y++) {
            uint16_t mask = 0x8000;
            for (uint8_t x = 0; x < 16; x++) {
                if (bitmap[y] & mask) {
                    // Map to coordinates with center (0,0,0) scaled by radius
                    Vector3 point = Vector3(x - 7.5f, 4.5f - y, 0) / 7.5f * radius;
                    // Project to a line at the bottom of the coordinate system
                    Vector3 line = Vector3(point.x, -radius, 0);
                    // Rotation around X-axis with increased rotation by y offset
                    Quaternion q1 = Quaternion(angle - (arc * point.y), Vector3(1, 0, 0));

                    // Apply rotations and draw with scaled colors
                    Color c = Color((hue16 >> 8) + 0 + 8 * y, RainbowGradientPalette);
                    c.scale(brightness);
                    radiate((q2 * q1).rotate(line * 0.8f), c, distance);

                    c = Color((hue16 >> 8) + 64 + 8 * y, RainbowGradientPalette);
                    c.scale(brightness);
                    radiate((q3 * q1).rotate(line * 0.9f), c, distance);

                    c = Color((hue16 >> 8) + 128 + 8 * y, RainbowGradientPalette);
                    c.scale(brightness);
                    radiate((q4 * q1).rotate(line * 1.0f), c, distance);
                }
                mask >>= 1;
            }
        }
    }
};

//=============================================================================
// Demo 11: Mario (Direct port from real cube)
//=============================================================================
#include "gfx/Mario.h"

class MarioDemo : public DemoAnimation {
    float angle = 0;
    float angle_speed = 1.5f;
    float radius = 7.0f;

    float frame_timer = 0;
    float frame_interval = 0.15f;
    uint8_t frame = 0;
    uint8_t frame_display[6] = {0, 1, 2, 3, 2, 1};

public:
    void init() override {
        angle = 0;
        frame = 0;
        frame_timer = 0;
    }

    const char* name() override { return "Mario (Real Port)"; }

    void update(float dt) override {
        angle += dt * angle_speed;

        frame_timer += dt;
        if (frame_timer >= frame_interval) {
            frame_timer = 0;
            if (++frame >= 6) frame = 0;
        }

        float arc = 2 * (180 / PI) * asinf(0.5f / radius);

        for (uint8_t y = 0; y < FRAME_HEIGHT; y++) {
            for (uint8_t x = 0; x < FRAME_WIDTH; x++) {
                uint32_t data = mario_data[frame_display[frame]][y * 16 + x];
                // Default to black
                Color c = Color(0, 0, 0);
                if (data >> 24 & 0xff) {  // If alpha channel is set
                    c = Color(data & 0xff, data >> 8 & 0xff, data >> 16 & 0xff);
                    if (c.isBlack()) {
                        c = Color(0, 0, 0);
                    } else {
                        c.gamma();
                    }
                }
                // Map to coordinates - project on cylinder at left of coordinate system
                Vector3 point = Vector3(-radius, (CY - y) / CY * radius, 0);
                Quaternion q = Quaternion(angle - (arc * x), Vector3(0, 1, 0));
                radiate(q.rotate(point), c, 1.0f);
            }
        }
    }
};

//=============================================================================
// Demo 12: Scroller (Direct port from real cube)
//=============================================================================
#include "gfx/Charset.h"

class ScrollerDemo : public DemoAnimation {
    float radius = 7.0f;
    float text_rotation = -100.0f;
    float text_rotation_speed = 50.0f;
    const char* text = "MEGA CUBE 16x16x16 ";
    uint8_t brightness = 255;

public:
    void init() override {
        text_rotation = -100.0f;
    }

    const char* name() override { return "Scroller (Real Port)"; }

    uint16_t match_char(char chr) {
        if (chr >= ' ' && chr <= '~')
            return chr - ' ';
        else
            return '#' - ' ';
    }

    void update(float dt) override {
        text_rotation += text_rotation_speed * dt;

        // Full circle resolution in pixels
        float circle_resolution = 2 * PI * radius;
        // Angle adjustment in degrees for each line
        float line_angle_adj = 360.0f / circle_resolution;
        // Amount of pixels the text has been rotated
        float pixel_start = text_rotation / line_angle_adj;
        // Start of the line_angle at a bit over 90 degrees
        float line_angle = 100.0f;

        int16_t pixel_line = (int16_t)pixel_start;
        if (pixel_line < 0) {
            line_angle += line_angle_adj * pixel_line;
            pixel_line = 0;
        }

        const uint16_t blank_lines = 1;
        size_t text_len = strlen(text);
        uint16_t text_lines = (CHARSET_FRAME_HEIGHT + blank_lines) * (uint16_t)text_len;

        while (line_angle > -line_angle_adj) {
            uint16_t text_offset = pixel_line % text_lines;
            uint16_t char_offset = text_offset / (CHARSET_FRAME_HEIGHT + blank_lines);
            uint16_t t = match_char(text[char_offset]);
            uint16_t y = pixel_line++ % (CHARSET_FRAME_HEIGHT + blank_lines);

            if (y < CHARSET_FRAME_HEIGHT) {
                Quaternion q = Quaternion(line_angle, Vector3(1, 0, 0));
                for (uint8_t x = 0; x < CHARSET_FRAME_WIDTH; x++) {
                    uint32_t data = charset_data[t][y * CHARSET_FRAME_WIDTH + x];
                    if (data & 0xff000000) {
                        Color c = Color(data & 0xff, data >> 8 & 0xff, data >> 16 & 0xff);
                        Vector3 pixel = q.rotate(
                            Vector3(x / (CHARSET_FRAME_WIDTH - 1.0f), 0, -1) * radius);
                        pixel = pixel + Vector3(-radius / 2.0f, -radius / 2.0f, radius / 2.0f);
                        voxel(pixel, c.scale(brightness).gamma());
                    }
                }
            }
            line_angle -= line_angle_adj;
        }
    }
};

//=============================================================================
// Demo 13: Rain
//=============================================================================
class RainDemo : public DemoAnimation {
    struct Drop { float x, y, z, speed; bool active; };
    struct Splash { float x, y, z, vx, vy, vz, life; };
    static const int MAX_DROPS = 80;
    static const int MAX_SPLASHES = 150;
    Drop drops[MAX_DROPS];
    Splash splashes[MAX_SPLASHES];
    int numSplashes = 0;
    float spawnTimer = 0;

public:
    void init() override {
        for (int i = 0; i < MAX_DROPS; i++) drops[i].active = false;
        numSplashes = 0;
        spawnTimer = 0;
    }

    const char* name() override { return "Rain"; }

    void update(float dt) override {
        spawnTimer += dt;
        if (spawnTimer > 0.02f) {
            spawnTimer = 0;
            for (int i = 0; i < MAX_DROPS; i++) {
                if (!drops[i].active) {
                    drops[i].x = noise.nextRandom(-7.5f, 7.5f);
                    drops[i].z = noise.nextRandom(-7.5f, 7.5f);
                    drops[i].y = 7.5f;
                    drops[i].speed = noise.nextRandom(15, 25);
                    drops[i].active = true;
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_DROPS; i++) {
            if (!drops[i].active) continue;
            drops[i].y -= drops[i].speed * dt;
            if (drops[i].y <= -7.5f) {
                for (int j = 0; j < 4 && numSplashes < MAX_SPLASHES; j++) {
                    Splash& s = splashes[numSplashes++];
                    s.x = drops[i].x; s.y = -7.5f; s.z = drops[i].z;
                    s.vx = noise.nextRandom(-3, 3);
                    s.vy = noise.nextRandom(3, 8);
                    s.vz = noise.nextRandom(-3, 3);
                    s.life = 1.0f;
                }
                drops[i].active = false;
            } else {
                voxel(Vector3(drops[i].x, drops[i].y, drops[i].z), Color(80, 130, 255));
            }
        }

        for (int i = 0; i < numSplashes; ) {
            Splash& s = splashes[i];
            s.x += s.vx * dt; s.y += s.vy * dt; s.z += s.vz * dt;
            s.vy -= 20.0f * dt;
            s.life -= dt * 2.5f;
            if (s.life <= 0 || s.y < -7.5f) {
                splashes[i] = splashes[--numSplashes];
            } else {
                Color c(150, 200, 255);
                c.scale((uint8_t)(s.life * 255));
                voxel(Vector3(s.x, s.y, s.z), c);
                i++;
            }
        }
    }
};

//=============================================================================
// Demo 14: Aurora Borealis
//=============================================================================
class AuroraDemo : public DemoAnimation {
    float phase = 0;

public:
    void init() override { phase = 0; }
    const char* name() override { return "Aurora Borealis"; }

    void update(float dt) override {
        phase += dt * 0.3f;
        for (int x = 0; x < 16; x++) {
            for (int z = 0; z < 16; z++) {
                float h1 = 10 + 4 * noise.noise3(x * 0.15f + phase, z * 0.15f, phase * 0.5f);
                float h2 = 8 + 3 * noise.noise3(x * 0.2f, z * 0.2f + phase * 0.7f, phase * 0.3f);
                for (int y = 0; y < 16; y++) {
                    float b1 = fmaxf(0.0f, 1.0f - fabsf(y - h1) * 0.4f);
                    float b2 = fmaxf(0.0f, 0.7f - fabsf(y - h2) * 0.35f);
                    float brightness = b1 + b2;
                    if (brightness > 0.05f) {
                        brightness = fminf(brightness, 1.0f);
                        float t = y / 15.0f;
                        uint8_t r = (uint8_t)(t * 150 * brightness);
                        uint8_t g = (uint8_t)((1.0f - t * 0.5f) * 255 * brightness);
                        uint8_t b = (uint8_t)(t * 200 * brightness);
                        voxel((uint8_t)x, (uint8_t)y, (uint8_t)z, Color(r, g, b));
                    }
                }
            }
        }
    }
};

//=============================================================================
// Demo 15: Lava Lamp
//=============================================================================
class LavaLampDemo : public DemoAnimation {
    struct Blob { float phaseY, phaseX, phaseZ, speedY, speedX, speedZ, size; };
    static const int NUM_BLOBS = 5;
    Blob blobs[NUM_BLOBS];
    float phase = 0;

public:
    void init() override {
        phase = 0;
        for (int i = 0; i < NUM_BLOBS; i++) {
            blobs[i].phaseY = noise.nextRandom(0, TWO_PI);
            blobs[i].phaseX = noise.nextRandom(0, TWO_PI);
            blobs[i].phaseZ = noise.nextRandom(0, TWO_PI);
            blobs[i].speedY = noise.nextRandom(0.15f, 0.35f);
            blobs[i].speedX = noise.nextRandom(0.08f, 0.2f);
            blobs[i].speedZ = noise.nextRandom(0.08f, 0.2f);
            blobs[i].size = noise.nextRandom(2.2f, 3.2f);
        }
    }

    const char* name() override { return "Lava Lamp"; }

    void update(float dt) override {
        phase += dt;

        // Blob centers - drift through full cube space
        float bx[NUM_BLOBS], by[NUM_BLOBS], bz[NUM_BLOBS];
        for (int i = 0; i < NUM_BLOBS; i++) {
            by[i] = sinf(phase * blobs[i].speedY + blobs[i].phaseY) * 6.5f;
            bx[i] = sinf(phase * blobs[i].speedX + blobs[i].phaseX) * 4.0f;
            bz[i] = sinf(phase * blobs[i].speedZ + blobs[i].phaseZ) * 4.0f;
        }

        float threshold = 1.0f;
        for (int vx = 0; vx < 16; vx++) {
            for (int vy = 0; vy < 16; vy++) {
                for (int vz = 0; vz < 16; vz++) {
                    float fx = vx - 7.5f, fy = vy - 7.5f, fz = vz - 7.5f;

                    // Metaball field from blobs - they merge when close
                    float field = 0;
                    for (int i = 0; i < NUM_BLOBS; i++) {
                        float dx = fx - bx[i], dy = fy - by[i], dz = fz - bz[i];
                        float r2 = blobs[i].size * blobs[i].size;
                        field += r2 / (dx*dx + dy*dy + dz*dz + 0.5f);
                    }

                    // Thin wax pool settled at the bottom (1-2 voxels thick)
                    if (fy < -5.5f) {
                        float poolDy = fy + 7.5f; // distance from very bottom
                        float poolField = 1.5f / (poolDy * poolDy + 0.5f);
                        field += poolField;
                    }

                    if (field > threshold) {
                        // Depth into blob: 0 at surface, slow ramp to 1 deep inside
                        float depth = fminf((field - threshold) * 0.6f, 1.0f);
                        // Surface = deep red, core = bright yellow-white
                        uint8_t r = (uint8_t)(100 + 155 * depth);
                        uint8_t g = (uint8_t)(20 + 180 * depth * depth);
                        uint8_t b = (uint8_t)(5 + 60 * depth * depth * depth);
                        voxel((uint8_t)vx, (uint8_t)vy, (uint8_t)vz, Color(r, g, b));
                    } else {
                        // Dim liquid fill across entire cube
                        float glow = 0.04f + 0.02f * sinf(fy * 0.3f + phase * 0.1f);
                        glow += field * 0.04f;
                        uint8_t r = (uint8_t)(100 * glow);
                        uint8_t g = (uint8_t)(25 * glow);
                        uint8_t b = (uint8_t)(8 * glow);
                        if (r > 2)
                            voxel((uint8_t)vx, (uint8_t)vy, (uint8_t)vz, Color(r, g, b));
                    }
                }
            }
        }
    }
};

//=============================================================================
// Demo 16: Fireflies
//=============================================================================
class FirefliesDemo : public DemoAnimation {
    struct Firefly { float x, y, z, vx, vy, vz, phase, speed; uint8_t hue; };
    static const int NUM_FLIES = 40;
    Firefly flies[NUM_FLIES];

public:
    void init() override {
        for (int i = 0; i < NUM_FLIES; i++) {
            flies[i].x = noise.nextRandom(-7, 7);
            flies[i].y = noise.nextRandom(-7, 7);
            flies[i].z = noise.nextRandom(-7, 7);
            flies[i].vx = noise.nextRandom(-1, 1);
            flies[i].vy = noise.nextRandom(-1, 1);
            flies[i].vz = noise.nextRandom(-1, 1);
            flies[i].phase = noise.nextRandom(0, TWO_PI);
            flies[i].speed = noise.nextRandom(0.5f, 2.0f);
            flies[i].hue = (uint8_t)(rand() % 256);
        }
    }

    const char* name() override { return "Fireflies"; }

    void update(float dt) override {
        for (int i = 0; i < NUM_FLIES; i++) {
            Firefly& f = flies[i];
            f.phase += dt * f.speed;
            float brightness = fmaxf(0.0f, sinf(f.phase));
            brightness *= brightness;

            f.x += f.vx * dt; f.y += f.vy * dt; f.z += f.vz * dt;
            f.vx += noise.nextRandom(-2, 2) * dt;
            f.vy += noise.nextRandom(-2, 2) * dt;
            f.vz += noise.nextRandom(-2, 2) * dt;
            float speed = sqrtf(f.vx*f.vx + f.vy*f.vy + f.vz*f.vz);
            if (speed > 2.0f) { f.vx *= 2.0f/speed; f.vy *= 2.0f/speed; f.vz *= 2.0f/speed; }

            if (f.x < -7 || f.x > 7) f.vx = -f.vx;
            if (f.y < -7 || f.y > 7) f.vy = -f.vy;
            if (f.z < -7 || f.z > 7) f.vz = -f.vz;
            f.x = fmaxf(-7.5f, fminf(7.5f, f.x));
            f.y = fmaxf(-7.5f, fminf(7.5f, f.y));
            f.z = fmaxf(-7.5f, fminf(7.5f, f.z));

            if (brightness > 0.05f) {
                Color c = Color(f.hue, RainbowGradientPalette);
                c.scale((uint8_t)(brightness * 255));
                radiate5(Vector3(f.x, f.y, f.z), c, 2.0f);
            }
        }
    }
};

//=============================================================================
// Demo 17: Ocean Waves
//=============================================================================
class OceanDemo : public DemoAnimation {
    float phase = 0;

public:
    void init() override { phase = 0; }
    const char* name() override { return "Ocean Waves"; }

    void update(float dt) override {
        phase += dt;
        for (int x = 0; x < 16; x++) {
            for (int z = 0; z < 16; z++) {
                float fx = (x - 7.5f) / 7.5f;
                float fz = (z - 7.5f) / 7.5f;
                float h = sinf(fx * 3 + phase * 2) * 0.3f
                        + sinf(fz * 4 + phase * 1.5f) * 0.25f
                        + sinf((fx + fz) * 2.5f + phase * 1.8f) * 0.2f;
                int surfaceY = (int)(7.5f + h * 5);
                if (surfaceY > 15) surfaceY = 15;

                for (int y = 0; y <= surfaceY && y < 16; y++) {
                    float depthRatio = (float)(surfaceY - y) / (float)(surfaceY + 1);
                    float n = noise.noise3(x * 0.25f, y * 0.3f, z * 0.25f + phase * 0.3f) * 0.3f;

                    if (y >= surfaceY - 1) {
                        // Foam/crest - bright white-blue
                        float foam = 0.7f + 0.3f * sinf(fx * 5 + fz * 3 + phase * 3);
                        voxel((uint8_t)x, (uint8_t)y, (uint8_t)z,
                              Color((uint8_t)(180*foam), (uint8_t)(220*foam), 255));
                    } else if (depthRatio < 0.3f) {
                        // Shallow - light blue-green
                        float t = depthRatio / 0.3f;
                        uint8_t r = (uint8_t)((30 - 20*t + n*30));
                        uint8_t g = (uint8_t)((160 - 40*t + n*40));
                        uint8_t b = (uint8_t)((220 - 20*t + n*20));
                        voxel((uint8_t)x, (uint8_t)y, (uint8_t)z, Color(r, g, b));
                    } else if (depthRatio < 0.6f) {
                        // Mid depth - medium blue
                        float t = (depthRatio - 0.3f) / 0.3f;
                        uint8_t r = (uint8_t)(10 - 5*t);
                        uint8_t g = (uint8_t)((120 - 50*t + n*30));
                        uint8_t b = (uint8_t)((200 - 30*t + n*20));
                        voxel((uint8_t)x, (uint8_t)y, (uint8_t)z, Color(r, g, b));
                    } else {
                        // Deep - dark navy/black
                        float t = (depthRatio - 0.6f) / 0.4f;
                        uint8_t r = (uint8_t)(5 * (1-t));
                        uint8_t g = (uint8_t)((70 - 50*t + n*20));
                        uint8_t b = (uint8_t)((170 - 100*t + n*20));
                        voxel((uint8_t)x, (uint8_t)y, (uint8_t)z, Color(r, g, b));
                    }
                }
            }
        }
    }
};

//=============================================================================
// Demo 18: Lorenz Attractor
//=============================================================================
class LorenzDemo : public DemoAnimation {
    static const int TRAIL_LEN = 500;
    Vector3 trail[TRAIL_LEN];
    int trailHead = 0;
    int trailCount = 0;
    float lx, ly, lz;
    float angle = 0;
    float sigma = 10.0f, rho = 28.0f, beta = 8.0f / 3.0f;

public:
    void init() override {
        lx = 1; ly = 1; lz = 1;
        trailHead = 0; trailCount = 0;
        angle = 0;
    }

    const char* name() override { return "Lorenz Attractor"; }

    void update(float dt) override {
        angle += dt * 0.2f;
        Quaternion q = Quaternion(angle * 30, Vector3(0, 1, 0));

        float step = 0.005f;
        int steps = (int)(dt / step) + 1;
        for (int s = 0; s < steps; s++) {
            float dx = sigma * (ly - lx) * step;
            float dy = (lx * (rho - lz) - ly) * step;
            float dz = (lx * ly - beta * lz) * step;
            lx += dx; ly += dy; lz += dz;

            Vector3 p(lx / 20.0f * 6, (lz - 25) / 25.0f * 6, ly / 20.0f * 6);
            trail[trailHead] = p;
            trailHead = (trailHead + 1) % TRAIL_LEN;
            if (trailCount < TRAIL_LEN) trailCount++;
        }

        for (int i = 0; i < trailCount; i++) {
            int idx = (trailHead - trailCount + i + TRAIL_LEN) % TRAIL_LEN;
            float t = (float)i / trailCount;
            Color c = Color((uint8_t)(t * 255), RainbowGradientPalette);
            c.scale((uint8_t)(t * 255));
            voxel(q.rotate(trail[idx]), c);
        }
    }
};

//=============================================================================
// Demo 19: Torus
//=============================================================================
class TorusDemo : public DemoAnimation {
    float angle = 0;
    uint16_t hue16 = 0;

public:
    void init() override { angle = 0; hue16 = 0; }
    const char* name() override { return "Torus"; }

    void update(float dt) override {
        angle += dt;
        hue16 += (uint16_t)(dt * 40 * 255);
        float R = 4.5f, r = 2.0f;
        Quaternion q = Quaternion(angle * 30, Vector3(1, 0, 0))
                     * Quaternion(angle * 20, Vector3(0, 0, 1));

        int resU = 40, resV = 20;
        for (int i = 0; i < resU; i++) {
            float u = (float)i / resU * TWO_PI;
            for (int j = 0; j < resV; j++) {
                float v = (float)j / resV * TWO_PI;
                Vector3 p((R + r * cosf(v)) * cosf(u),
                          (R + r * cosf(v)) * sinf(u),
                          r * sinf(v));
                Color c = Color((uint8_t)((hue16 >> 8) + i * 5), RainbowGradientPalette);
                radiate(q.rotate(p), c, 1.2f);
            }
        }
    }
};

//=============================================================================
// Demo 20: Sierpinski Tetrahedron
//=============================================================================
class SierpinskiDemo : public DemoAnimation {
    static const int NUM_POINTS = 2000;
    Vector3 points[NUM_POINTS];
    float angle = 0;
    uint16_t hue16 = 0;
    Vector3 vertices[4] = {
        Vector3(1, 1, 1), Vector3(1, -1, -1),
        Vector3(-1, 1, -1), Vector3(-1, -1, 1)
    };

public:
    void init() override {
        angle = 0; hue16 = 0;
        Vector3 p(0, 0, 0);
        for (int i = 0; i < NUM_POINTS; i++) {
            p = (p + vertices[rand() % 4] * 6.0f) * 0.5f;
            points[i] = p;
        }
    }

    const char* name() override { return "Sierpinski Tetrahedron"; }

    void update(float dt) override {
        angle += dt * 0.5f;
        hue16 += (uint16_t)(dt * 30 * 255);
        Quaternion q = Quaternion(angle * 30, Vector3(0, 1, 0));
        for (int i = 0; i < NUM_POINTS; i++) {
            Color c = Color((uint8_t)((hue16 >> 8) + (uint8_t)(points[i].y * 15)), RainbowGradientPalette);
            voxel(q.rotate(points[i]), c);
        }
    }
};

//=============================================================================
// Demo 21: Moebius Strip
//=============================================================================
class MoebiusDemo : public DemoAnimation {
    float angle = 0;
    uint16_t hue16 = 0;

public:
    void init() override { angle = 0; hue16 = 0; }
    const char* name() override { return "Moebius Strip"; }

    void update(float dt) override {
        angle += dt;
        hue16 += (uint16_t)(dt * 40 * 255);
        Quaternion q = Quaternion(angle * 25, Vector3(0, 1, 0));
        float R = 5.0f, width = 2.5f;
        int resT = 60, resS = 8;

        for (int i = 0; i < resT; i++) {
            float t = (float)i / resT * TWO_PI;
            for (int j = 0; j < resS; j++) {
                float s = ((float)j / (resS - 1) - 0.5f) * width;
                float x = (R + s * cosf(t / 2)) * cosf(t);
                float y = s * sinf(t / 2);
                float z = (R + s * cosf(t / 2)) * sinf(t);
                Color c = Color((uint8_t)((hue16 >> 8) + i * 3), RainbowGradientPalette);
                radiate(q.rotate(Vector3(x, y, z)), c, 1.0f);
            }
        }
    }
};

//=============================================================================
// Demo 22: Spirograph 3D
//=============================================================================
class SpirographDemo : public DemoAnimation {
    float phase = 0;
    static const int TRAIL_LEN = 600;
    Vector3 trail[TRAIL_LEN];
    int trailCount = 0;
    int trailHead = 0;

public:
    void init() override { phase = 0; trailCount = 0; trailHead = 0; }
    const char* name() override { return "Spirograph 3D"; }

    void update(float dt) override {
        phase += dt;
        float t = phase * 2;
        float a = 3 + sinf(phase * 0.1f);
        float b = 2 + cosf(phase * 0.13f);
        float cp = 5 + sinf(phase * 0.07f);
        float d = 3 + cosf(phase * 0.11f);

        float x = sinf(a * t) * cosf(b * t) * 6.5f;
        float y = sinf(cp * t) * 6.5f;
        float z = cosf(d * t) * sinf(b * t) * 6.5f;

        trail[trailHead] = Vector3(x, y, z);
        trailHead = (trailHead + 1) % TRAIL_LEN;
        if (trailCount < TRAIL_LEN) trailCount++;

        for (int i = 0; i < trailCount; i++) {
            int idx = (trailHead - trailCount + i + TRAIL_LEN) % TRAIL_LEN;
            float frac = (float)i / trailCount;
            Color col = Color((uint8_t)(frac * 255), RainbowGradientPalette);
            col.scale((uint8_t)(frac * 255));
            voxel(trail[idx], col);
        }
    }
};

//=============================================================================
// Demo 23: Snake 3D (Self-Playing)
//=============================================================================
class SnakeDemo : public DemoAnimation {
    static const int MAX_LEN = 200;
    struct Pos { int8_t x, y, z; };
    Pos body[MAX_LEN];
    int length = 0;
    int headIdx = 0;
    Pos food;
    float moveTimer = 0;
    float moveInterval = 0.1f;
    uint16_t hue16 = 0;
    bool grid[16][16][16];

    void placeFood() {
        for (int attempt = 0; attempt < 100; attempt++) {
            food.x = rand() % 16; food.y = rand() % 16; food.z = rand() % 16;
            if (!grid[food.x][food.y][food.z]) return;
        }
    }

    bool isValid(int x, int y, int z) {
        return x >= 0 && x < 16 && y >= 0 && y < 16 && z >= 0 && z < 16 && !grid[x][y][z];
    }

public:
    void init() override {
        memset(grid, 0, sizeof(grid));
        length = 3; headIdx = 2;
        for (int i = 0; i < 3; i++) {
            body[i] = {8, 8, (int8_t)(8 + i)};
            grid[8][8][8 + i] = true;
        }
        hue16 = 0; moveTimer = 0;
        placeFood();
    }

    const char* name() override { return "Snake 3D"; }

    void update(float dt) override {
        hue16 += (uint16_t)(dt * 30 * 255);
        moveTimer += dt;

        if (moveTimer >= moveInterval) {
            moveTimer = 0;
            Pos head = body[headIdx];
            int dirs[6][3] = {{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
            int bestDir = -1;
            float bestDist = 1e9f;

            for (int d = 0; d < 6; d++) {
                int nx = head.x + dirs[d][0], ny = head.y + dirs[d][1], nz = head.z + dirs[d][2];
                if (isValid(nx, ny, nz)) {
                    float dist = (float)((nx-food.x)*(nx-food.x) + (ny-food.y)*(ny-food.y) + (nz-food.z)*(nz-food.z));
                    if (dist < bestDist) { bestDist = dist; bestDir = d; }
                }
            }

            if (bestDir == -1) { init(); return; }

            Pos newHead = {(int8_t)(head.x + dirs[bestDir][0]),
                           (int8_t)(head.y + dirs[bestDir][1]),
                           (int8_t)(head.z + dirs[bestDir][2])};
            bool ate = (newHead.x == food.x && newHead.y == food.y && newHead.z == food.z);

            if (!ate) {
                int tailIdx = (headIdx - length + 1 + MAX_LEN) % MAX_LEN;
                grid[body[tailIdx].x][body[tailIdx].y][body[tailIdx].z] = false;
            } else {
                length++;
                if (length >= MAX_LEN) { init(); return; }
                placeFood();
            }

            headIdx = (headIdx + 1) % MAX_LEN;
            body[headIdx] = newHead;
            grid[newHead.x][newHead.y][newHead.z] = true;
        }

        for (int i = 0; i < length; i++) {
            int idx = (headIdx - length + 1 + i + MAX_LEN) % MAX_LEN;
            float t = (float)i / length;
            Color c = Color((uint8_t)((hue16 >> 8) + (uint8_t)(t * 128)), RainbowGradientPalette);
            voxel((uint8_t)body[idx].x, (uint8_t)body[idx].y, (uint8_t)body[idx].z, c);
        }

        float blink = sinf(hue16 / 500.0f) * 0.5f + 0.5f;
        voxel((uint8_t)food.x, (uint8_t)food.y, (uint8_t)food.z,
              Color(255, (uint8_t)(255 * blink), 0));
    }
};

//=============================================================================
// Demo 24: Tetris 3D (Self-Playing)
//=============================================================================
class TetrisDemo : public DemoAnimation {
    bool board[16][16][16];
    Color boardColors[16][16][16];
    struct Piece { int8_t blocks[4][3]; };
    static const int NUM_TYPES = 5;
    int pieceX, pieceY, pieceZ;
    Piece cur;
    Color pieceColor;
    float fallTimer = 0;
    float fallInterval = 0.25f;
    uint16_t hue16 = 0;

    Piece types[NUM_TYPES] = {
        {{{0,0,0},{1,0,0},{0,0,1},{1,0,1}}},
        {{{0,0,0},{1,0,0},{2,0,0},{0,0,1}}},
        {{{0,0,0},{1,0,0},{0,1,0},{0,0,1}}},
        {{{0,0,0},{1,0,0},{2,0,0},{3,0,0}}},
        {{{0,0,0},{1,0,0},{1,0,1},{0,0,1}}}
    };

    void spawn() {
        cur = types[rand() % NUM_TYPES];
        pieceX = 5 + rand() % 6; pieceY = 14; pieceZ = 5 + rand() % 6;
        pieceColor = Color((uint8_t)(rand() % 256), RainbowGradientPalette);
    }

    bool fits(int px, int py, int pz) {
        for (int i = 0; i < 4; i++) {
            int x = px + cur.blocks[i][0], y = py + cur.blocks[i][1], z = pz + cur.blocks[i][2];
            if (x < 0 || x >= 16 || y < 0 || y >= 16 || z < 0 || z >= 16) return false;
            if (board[x][y][z]) return false;
        }
        return true;
    }

    void lock() {
        for (int i = 0; i < 4; i++) {
            int x = pieceX + cur.blocks[i][0], y = pieceY + cur.blocks[i][1], z = pieceZ + cur.blocks[i][2];
            if (x >= 0 && x < 16 && y >= 0 && y < 16 && z >= 0 && z < 16) {
                board[x][y][z] = true;
                boardColors[x][y][z] = pieceColor;
            }
        }
    }

    void clearLayers() {
        for (int y = 0; y < 16; y++) {
            bool full = true;
            for (int x = 0; x < 16 && full; x++)
                for (int z = 0; z < 16 && full; z++)
                    if (!board[x][y][z]) full = false;
            if (full) {
                for (int yy = y; yy < 15; yy++)
                    for (int x = 0; x < 16; x++)
                        for (int z = 0; z < 16; z++) {
                            board[x][yy][z] = board[x][yy+1][z];
                            boardColors[x][yy][z] = boardColors[x][yy+1][z];
                            board[x][yy+1][z] = false;
                        }
                y--;
            }
        }
    }

public:
    void init() override {
        memset(board, 0, sizeof(board));
        hue16 = 0; fallTimer = 0;
        spawn();
    }

    const char* name() override { return "Tetris 3D"; }

    void update(float dt) override {
        hue16 += (uint16_t)(dt * 20 * 255);
        fallTimer += dt;

        if (fallTimer >= fallInterval) {
            fallTimer = 0;
            if (fits(pieceX, pieceY - 1, pieceZ)) {
                pieceY--;
            } else {
                lock();
                clearLayers();
                bool gameOver = false;
                for (int x = 0; x < 16 && !gameOver; x++)
                    for (int z = 0; z < 16 && !gameOver; z++)
                        if (board[x][14][z]) gameOver = true;
                if (gameOver) { init(); return; }
                spawn();
            }
        }

        for (int x = 0; x < 16; x++)
            for (int y = 0; y < 16; y++)
                for (int z = 0; z < 16; z++)
                    if (board[x][y][z])
                        voxel((uint8_t)x, (uint8_t)y, (uint8_t)z, boardColors[x][y][z]);

        for (int i = 0; i < 4; i++) {
            int x = pieceX + cur.blocks[i][0], y = pieceY + cur.blocks[i][1], z = pieceZ + cur.blocks[i][2];
            if (x >= 0 && x < 16 && y >= 0 && y < 16 && z >= 0 && z < 16)
                voxel((uint8_t)x, (uint8_t)y, (uint8_t)z, pieceColor);
        }
    }
};

//=============================================================================
// Demo 25: Pong 3D (Self-Playing)
//=============================================================================
class PongDemo : public DemoAnimation {
    float ballX, ballY, ballZ, ballVX, ballVY, ballVZ;
    float p1X, p1Z, p2X, p2Z;
    float padSize = 3.0f;

    void resetBall() {
        ballX = 7.5f; ballY = 7.5f; ballZ = 7.5f;
        ballVX = noise.nextRandom(-5, 5);
        ballVY = (rand() % 2 ? 1 : -1) * noise.nextRandom(8, 12);
        ballVZ = noise.nextRandom(-5, 5);
    }

public:
    void init() override {
        p1X = p2X = 7.5f; p1Z = p2Z = 7.5f;
        resetBall();
    }

    const char* name() override { return "Pong 3D"; }

    void update(float dt) override {
        // AI paddle tracking (slightly imperfect - adds wobble)
        float spd = 8.0f;
        float targetX1 = ballX + noise.nextRandom(-1.5f, 1.5f);
        float targetZ1 = ballZ + noise.nextRandom(-1.5f, 1.5f);
        float targetX2 = ballX + noise.nextRandom(-1.5f, 1.5f);
        float targetZ2 = ballZ + noise.nextRandom(-1.5f, 1.5f);
        p1X += fminf(fabsf(targetX1 - p1X), spd * dt) * (targetX1 > p1X ? 1 : -1);
        p1Z += fminf(fabsf(targetZ1 - p1Z), spd * dt) * (targetZ1 > p1Z ? 1 : -1);
        p2X += fminf(fabsf(targetX2 - p2X), spd * dt) * (targetX2 > p2X ? 1 : -1);
        p2Z += fminf(fabsf(targetZ2 - p2Z), spd * dt) * (targetZ2 > p2Z ? 1 : -1);
        p1X = fmaxf(padSize, fminf(15 - padSize, p1X));
        p1Z = fmaxf(padSize, fminf(15 - padSize, p1Z));
        p2X = fmaxf(padSize, fminf(15 - padSize, p2X));
        p2Z = fmaxf(padSize, fminf(15 - padSize, p2Z));

        ballX += ballVX * dt; ballY += ballVY * dt; ballZ += ballVZ * dt;
        if (ballX < 0 || ballX > 15) ballVX = -ballVX;
        if (ballZ < 0 || ballZ > 15) ballVZ = -ballVZ;
        ballX = fmaxf(0, fminf(15, ballX));
        ballZ = fmaxf(0, fminf(15, ballZ));

        // Deflection angle based on hit offset from paddle center
        if (ballY <= 1 && ballVY < 0) {
            if (fabsf(ballX - p1X) < padSize && fabsf(ballZ - p1Z) < padSize) {
                float offX = (ballX - p1X) / padSize;  // -1 to 1
                float offZ = (ballZ - p1Z) / padSize;
                ballVY = fabsf(ballVY);
                ballVX = offX * fabsf(ballVY) * 0.8f;  // deflect proportional to offset
                ballVZ = offZ * fabsf(ballVY) * 0.8f;
            } else if (ballY < 0) resetBall();
        }
        if (ballY >= 14 && ballVY > 0) {
            if (fabsf(ballX - p2X) < padSize && fabsf(ballZ - p2Z) < padSize) {
                float offX = (ballX - p2X) / padSize;
                float offZ = (ballZ - p2Z) / padSize;
                ballVY = -fabsf(ballVY);
                ballVX = offX * fabsf(ballVY) * 0.8f;
                ballVZ = offZ * fabsf(ballVY) * 0.8f;
            } else if (ballY > 15) resetBall();
        }

        for (int x = 0; x < 16; x++)
            for (int z = 0; z < 16; z++) {
                if (fabsf(x - p1X) < padSize && fabsf(z - p1Z) < padSize)
                    voxel((uint8_t)x, (uint8_t)0, (uint8_t)z, Color(50, 50, 255));
                if (fabsf(x - p2X) < padSize && fabsf(z - p2Z) < padSize)
                    voxel((uint8_t)x, (uint8_t)15, (uint8_t)z, Color(255, 50, 50));
            }

        radiate5(Vector3(ballX - CX, ballY - CY, ballZ - CZ), Color::WHITE, 2.5f);
    }
};

//=============================================================================
// Demo 26: Maze 3D (Generated & Solved)
//=============================================================================
class MazeDemo : public DemoAnimation {
    static const int SZ = 8;
    bool walls[SZ][SZ][SZ][6];
    bool visited[SZ][SZ][SZ];

    struct Pos { int8_t x, y, z; };
    Pos path[512];
    int pathLen = 0;
    int pathProgress = 0;
    float timer = 0;
    float doneTimer = 0;

    void generate() {
        memset(walls, true, sizeof(walls));
        memset(visited, false, sizeof(visited));
        int dirs[6][3] = {{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
        int opp[6] = {1,0,3,2,5,4};
        Pos stack[512]; int top = 0;
        stack[top++] = {0,0,0}; visited[0][0][0] = true;

        while (top > 0) {
            Pos cur = stack[top - 1];
            int valid[6], nv = 0;
            for (int d = 0; d < 6; d++) {
                int nx=cur.x+dirs[d][0], ny=cur.y+dirs[d][1], nz=cur.z+dirs[d][2];
                if (nx>=0 && nx<SZ && ny>=0 && ny<SZ && nz>=0 && nz<SZ && !visited[nx][ny][nz])
                    valid[nv++] = d;
            }
            if (nv == 0) { top--; continue; }
            int d = valid[rand() % nv];
            int nx=cur.x+dirs[d][0], ny=cur.y+dirs[d][1], nz=cur.z+dirs[d][2];
            walls[cur.x][cur.y][cur.z][d] = false;
            walls[nx][ny][nz][opp[d]] = false;
            visited[nx][ny][nz] = true;
            stack[top++] = {(int8_t)nx,(int8_t)ny,(int8_t)nz};
        }
    }

    void solve() {
        memset(visited, false, sizeof(visited));
        Pos parent[SZ][SZ][SZ];
        Pos queue[512]; int qh=0, qt=0;
        int dirs[6][3] = {{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
        queue[qt++] = {0,0,0}; visited[0][0][0] = true;

        while (qh < qt) {
            Pos cur = queue[qh++];
            if (cur.x==SZ-1 && cur.y==SZ-1 && cur.z==SZ-1) break;
            for (int d = 0; d < 6; d++) {
                if (walls[cur.x][cur.y][cur.z][d]) continue;
                int nx=cur.x+dirs[d][0], ny=cur.y+dirs[d][1], nz=cur.z+dirs[d][2];
                if (nx<0||nx>=SZ||ny<0||ny>=SZ||nz<0||nz>=SZ||visited[nx][ny][nz]) continue;
                visited[nx][ny][nz] = true;
                parent[nx][ny][nz] = cur;
                queue[qt++] = {(int8_t)nx,(int8_t)ny,(int8_t)nz};
            }
        }

        pathLen = 0;
        Pos p = {SZ-1,SZ-1,SZ-1};
        while (!(p.x==0 && p.y==0 && p.z==0) && pathLen < 512) {
            path[pathLen++] = p; p = parent[p.x][p.y][p.z];
        }
        path[pathLen++] = {0,0,0};
        for (int i = 0; i < pathLen/2; i++) {
            Pos t=path[i]; path[i]=path[pathLen-1-i]; path[pathLen-1-i]=t;
        }
    }

public:
    void init() override {
        generate(); solve();
        pathProgress = 0; timer = 0; doneTimer = 0;
    }

    const char* name() override { return "Maze 3D"; }

    void update(float dt) override {
        timer += dt;
        if (pathProgress < pathLen && timer > 0.2f) { timer = 0; pathProgress++; }
        if (pathProgress >= pathLen) { doneTimer += dt; if (doneTimer > 3.0f) init(); }

        // Draw path cells (2x2x2 each)
        for (int i = 0; i < pathProgress && i < pathLen; i++) {
            float t = (float)i / pathLen;
            Color c = Color((uint8_t)(t * 255), RainbowGradientPalette);
            int bx=path[i].x*2, by=path[i].y*2, bz=path[i].z*2;
            for (int a=0;a<2;a++) for(int b=0;b<2;b++) for(int d=0;d<2;d++)
                voxel((uint8_t)(bx+a),(uint8_t)(by+b),(uint8_t)(bz+d), c);
        }

        // Glow at solve head
        if (pathProgress > 0 && pathProgress <= pathLen) {
            int i = pathProgress - 1;
            radiate5(Vector3(path[i].x*2+0.5f-CX, path[i].y*2+0.5f-CY, path[i].z*2+0.5f-CZ),
                     Color::WHITE, 3.0f);
        }

        // Start/end markers
        radiate5(Vector3(0.5f-CX, 0.5f-CY, 0.5f-CZ), Color(50, 255, 50), 2.0f);
        radiate5(Vector3(SZ*2-1.5f-CX, SZ*2-1.5f-CY, SZ*2-1.5f-CZ), Color(255, 50, 50), 2.0f);
    }
};

//=============================================================================
// Demo 27: Globe
//=============================================================================
class GlobeDemo : public DemoAnimation {
    float angle = 0;

public:
    void init() override { angle = 0; }
    const char* name() override { return "Globe"; }

    void update(float dt) override {
        angle += dt * 0.3f;
        float radius = 7.0f;
        Quaternion q = Quaternion(angle * 20, Vector3(0, 1, 0));

        for (int x = 0; x < 16; x++) {
            for (int y = 0; y < 16; y++) {
                for (int z = 0; z < 16; z++) {
                    float fx=x-7.5f, fy=y-7.5f, fz=z-7.5f;
                    float dist = sqrtf(fx*fx + fy*fy + fz*fz);
                    if (dist > radius - 0.7f && dist < radius + 0.7f) {
                        Vector3 p = q.rotate(Vector3(fx, fy, fz));
                        float lat = asinf(p.y / dist);
                        float lon = atan2f(p.z, p.x);
                        float n = noise.noise2(lon * 1.5f + 10, lat * 3 + 10);
                        float absLat = fabsf(lat);

                        Color c;
                        if (absLat > 1.2f) c = Color(220, 230, 255);
                        else if (n > 0.45f) c = (absLat > 0.8f) ? Color(100, 130, 80) : Color(50, 160, 50);
                        else c = Color(30, 60, 200);

                        float edge = 1.0f - fabsf(dist - radius) / 0.7f;
                        c.scale((uint8_t)(edge * 255));
                        voxel((uint8_t)x, (uint8_t)y, (uint8_t)z, c);
                    }
                }
            }
        }
    }
};

//=============================================================================
// Demo 28: DNA Helix
//=============================================================================
class DNADemo : public DemoAnimation {
    float phase = 0;

public:
    void init() override { phase = 0; }
    const char* name() override { return "DNA Helix"; }

    void update(float dt) override {
        phase += dt;
        float helixR = 4.0f;
        Quaternion q = Quaternion(phase * 15, Vector3(0, 1, 0));
        int res = 60;

        for (int i = 0; i < res; i++) {
            float t = (float)i / res;
            float y = (t * 2 - 1) * 7.0f;
            float a = t * 4 * PI + phase * 2;

            Vector3 p1(cosf(a) * helixR, y, sinf(a) * helixR);
            Vector3 p2(cosf(a + PI) * helixR, y, sinf(a + PI) * helixR);
            radiate(q.rotate(p1), Color(50, 100, 255), 1.2f);
            radiate(q.rotate(p2), Color(255, 50, 50), 1.2f);

            if (i % 4 == 0) {
                for (int j = 0; j <= 6; j++) {
                    float f = (float)j / 6;
                    Vector3 rung = p1 * (1 - f) + p2 * f;
                    Color rc = (j < 3) ? Color(255, 200, 50) : Color(50, 255, 100);
                    radiate(q.rotate(rung), rc, 1.0f);
                }
            }
        }
    }
};

//=============================================================================
// Demo 29: Clock
//=============================================================================
class ClockDemo : public DemoAnimation {
    const uint8_t digits[10][5] = {
        {7,5,5,5,7},{2,6,2,2,7},{7,1,7,4,7},{7,1,7,1,7},{5,5,7,1,1},
        {7,4,7,1,7},{7,4,7,5,7},{7,1,1,1,1},{7,5,7,5,7},{7,5,7,1,7}
    };

    void drawDigit(int d, int ox, int oy, Color c) {
        for (int row = 0; row < 5; row++)
            for (int col = 0; col < 3; col++)
                if (digits[d][row] & (4 >> col))
                    for (int z = 6; z <= 9; z++)
                        voxel((uint8_t)(ox+col), (uint8_t)(oy-row), (uint8_t)z, c);
    }

public:
    void init() override {}
    const char* name() override { return "Clock"; }

    void update(float dt) override {
        time_t now = time(nullptr);
        struct tm* t = localtime(&now);

        drawDigit(t->tm_hour/10, 2, 14, Color(255, 60, 60));
        drawDigit(t->tm_hour%10, 10, 14, Color(255, 60, 60));

        if (t->tm_sec % 2 == 0) {
            for (int z = 7; z <= 8; z++) {
                voxel((uint8_t)7, (uint8_t)12, (uint8_t)z, Color::WHITE);
                voxel((uint8_t)7, (uint8_t)10, (uint8_t)z, Color::WHITE);
            }
        }

        drawDigit(t->tm_min/10, 2, 8, Color(60, 255, 60));
        drawDigit(t->tm_min%10, 10, 8, Color(60, 255, 60));

        int filled = (int)(t->tm_sec / 60.0f * 16);
        for (int x = 0; x < filled; x++)
            for (int z = 6; z <= 9; z++)
                voxel((uint8_t)x, (uint8_t)1, (uint8_t)z, Color(60, 60, 255));
    }
};

//=============================================================================
// Demo 30: Heart
//=============================================================================
class HeartDemo : public DemoAnimation {
    float phase = 0;

public:
    void init() override { phase = 0; }
    const char* name() override { return "Heart"; }

    void update(float dt) override {
        phase += dt;
        float scale = 1.0f + 0.1f * sinf(phase * 4);
        Quaternion q = Quaternion(phase * 15, Vector3(0, 1, 0));

        for (int vx = 0; vx < 16; vx++) {
            for (int vy = 0; vy < 16; vy++) {
                for (int vz = 0; vz < 16; vz++) {
                    // Rotate voxel into heart space
                    Vector3 p = q.rotate(Vector3(vx-7.5f, vy-7.5f, vz-7.5f));
                    float x = p.x / (5.0f * scale);
                    float y = p.y / (5.0f * scale);
                    float z = p.z / (5.0f * scale);

                    float a = x*x + 9.0f/4.0f*z*z + y*y - 1;
                    float val = a*a*a - x*x*y*y*y - 9.0f/200.0f*z*z*y*y*y;

                    if (val <= 0) {
                        float depth = fminf(-val * 5, 1.0f);
                        voxel((uint8_t)vx, (uint8_t)vy, (uint8_t)vz,
                              Color((uint8_t)(255*depth), (uint8_t)(20*depth), (uint8_t)(40*depth)));
                    }
                }
            }
        }
    }
};

//=============================================================================
// Demo 31: Hourglass
//=============================================================================
class HourglassDemo : public DemoAnimation {
    static const int MAX_SAND = 300;
    struct Sand { float x, y, z; bool falling; };
    Sand grains[MAX_SAND];
    int numGrains = 0;
    float flipTimer = 0;

public:
    void init() override {
        numGrains = 0; flipTimer = 0;
        for (int i = 0; i < MAX_SAND; i++) {
            float y = noise.nextRandom(9, 14);
            float maxR = (y - 7.5f) * 0.8f;
            float r = noise.nextRandom(0, fmaxf(0.5f, maxR));
            float a = noise.nextRandom(0, TWO_PI);
            grains[numGrains].x = 7.5f + r * cosf(a);
            grains[numGrains].y = y;
            grains[numGrains].z = 7.5f + r * sinf(a);
            grains[numGrains].falling = false;
            numGrains++;
        }
    }

    const char* name() override { return "Hourglass"; }

    void update(float dt) override {
        flipTimer += dt;
        if (flipTimer > 12.0f) { init(); return; }

        for (int i = 0; i < numGrains; i++) {
            Sand& s = grains[i];
            float cx = s.x - 7.5f, cz = s.z - 7.5f;
            float r = sqrtf(cx*cx + cz*cz);

            if (s.y > 7.5f) {
                if (r > 0.5f) { s.x += (7.5f - s.x) * dt * 2; s.z += (7.5f - s.z) * dt * 2; }
                if (r < 1.0f && s.y < 9.0f) { s.y -= dt * 8; s.falling = true; }
                else if (s.y > 8.0f) s.y -= dt * 0.5f;
            } else {
                if (s.falling) {
                    s.y -= dt * 8;
                    if (s.y < 2) { s.falling = false; s.x += noise.nextRandom(-1, 1); s.z += noise.nextRandom(-1, 1); }
                }
                s.y = fmaxf(0.5f, s.y);
            }
        }

        // Draw hourglass frame
        for (int y = 0; y < 16; y++) {
            float gy = fabsf(y - 7.5f);
            float r = gy * 0.7f + 0.5f;
            for (int a = 0; a < 32; a++) {
                float ang = a * TWO_PI / 32;
                int x = (int)(7.5f + r * cosf(ang));
                int z = (int)(7.5f + r * sinf(ang));
                if (x >= 0 && x < 16 && z >= 0 && z < 16)
                    voxel((uint8_t)x, (uint8_t)y, (uint8_t)z, Color(40, 40, 80));
            }
        }

        // Draw sand
        for (int i = 0; i < numGrains; i++) {
            int x=(int)grains[i].x, y=(int)grains[i].y, z=(int)grains[i].z;
            if (x>=0 && x<16 && y>=0 && y<16 && z>=0 && z<16)
                voxel((uint8_t)x, (uint8_t)y, (uint8_t)z, Color(220, 180, 80));
        }
    }
};

//=============================================================================
// Demo 32: Galaxy
//=============================================================================
class GalaxyDemo : public DemoAnimation {
    float angle = 0;
    static const int NUM_STARS = 600;
    struct Star { float r, armAngle, spread_x, spread_z, height; uint8_t arm; };
    Star stars[NUM_STARS];

public:
    void init() override {
        angle = 0;
        for (int i = 0; i < NUM_STARS; i++) {
            float t = noise.nextRandom(0.05f, 1.0f);
            stars[i].r = t * 7.0f;
            stars[i].arm = (uint8_t)(rand() % 3);
            stars[i].armAngle = t * 2.5f * PI;
            float armWidth = 0.5f + t * 1.5f;
            stars[i].spread_x = noise.nextGaussian(0, armWidth * 0.4f);
            stars[i].spread_z = noise.nextGaussian(0, armWidth * 0.4f);
            stars[i].height = noise.nextGaussian(0, 0.3f * (1 - t * 0.5f));
        }
    }

    const char* name() override { return "Galaxy"; }

    void update(float dt) override {
        angle += dt * 0.15f;
        Quaternion q = Quaternion(angle * 10, Vector3(0, 1, 0))
                     * Quaternion(35, Vector3(1, 0, 0));

        for (int i = 0; i < NUM_STARS; i++) {
            float a = stars[i].armAngle + stars[i].arm * TWO_PI / 3 + angle;
            float r = stars[i].r;
            float x = r * cosf(a) + stars[i].spread_x;
            float z = r * sinf(a) + stars[i].spread_z;
            float y = stars[i].height;

            float t = r / 7.0f;
            float brightness = 1.0f - t * 0.4f;
            // Inner stars warm white/yellow, outer stars bluer
            uint8_t cr = (uint8_t)((255 - t * 80) * brightness);
            uint8_t cg = (uint8_t)((230 - t * 60) * brightness);
            uint8_t cb = (uint8_t)((180 + t * 75) * brightness);
            voxel(q.rotate(Vector3(x, y, z)), Color(cr, cg, cb));
        }

        // Small bright core
        radiate5(q.rotate(Vector3(0, 0, 0)), Color(255, 240, 200), 2.0f);
    }
};

//=============================================================================
// Demo 33: Tornado
//=============================================================================
class TornadoDemo : public DemoAnimation {
    static const int NUM_PARTICLES = 300;
    struct TParticle { float angle, y, radius, speed; uint8_t hue; };
    TParticle parts[NUM_PARTICLES];
    float phase = 0;

public:
    void init() override {
        phase = 0;
        for (int i = 0; i < NUM_PARTICLES; i++) {
            parts[i].y = noise.nextRandom(-7.5f, 7.5f);
            parts[i].angle = noise.nextRandom(0, TWO_PI);
            float t = (parts[i].y + 7.5f) / 15.0f;
            parts[i].radius = 1.0f + t * 5.0f;
            parts[i].speed = noise.nextRandom(2, 5);
            parts[i].hue = (uint8_t)(rand() % 256);
        }
    }

    const char* name() override { return "Tornado"; }

    void update(float dt) override {
        phase += dt;
        for (int i = 0; i < NUM_PARTICLES; i++) {
            TParticle& p = parts[i];
            p.angle += p.speed * dt;
            p.y += dt * 3;
            if (p.y > 7.5f) {
                p.y = -7.5f;
                p.angle = noise.nextRandom(0, TWO_PI);
            }

            float t = (p.y + 7.5f) / 15.0f;
            float r = 1.0f + t * 5.0f;
            float x = r * cosf(p.angle);
            float z = r * sinf(p.angle);

            Color c = Color(p.hue, RainbowGradientPalette);
            c.scale((uint8_t)((1.0f - t * 0.3f) * 255));
            voxel(Vector3(x, p.y, z), c);
        }
    }
};

//=============================================================================
// Demo 34: Fountain
//=============================================================================
class FountainDemo : public DemoAnimation {
    static const int MAX_P = 300;
    struct FPart { float x, y, z, vx, vy, vz, life; uint8_t hue; };
    FPart parts[MAX_P];
    int numParts = 0;
    float spawnTimer = 0;
    uint16_t hue16 = 0;

public:
    void init() override { numParts = 0; spawnTimer = 0; hue16 = 0; }
    const char* name() override { return "Fountain"; }

    void update(float dt) override {
        hue16 += (uint16_t)(dt * 30 * 255);
        spawnTimer += dt;

        if (spawnTimer > 0.012f && numParts < MAX_P) {
            spawnTimer = 0;
            FPart& p = parts[numParts++];
            p.x = 0; p.y = -7.5f; p.z = 0;
            float a = noise.nextRandom(0, TWO_PI);
            float spread = noise.nextRandom(1, 4);
            p.vx = cosf(a) * spread;
            p.vy = noise.nextRandom(13, 17);
            p.vz = sinf(a) * spread;
            p.life = 1.0f;
            p.hue = (uint8_t)(hue16 >> 8);
        }

        for (int i = 0; i < numParts; ) {
            FPart& p = parts[i];
            p.x += p.vx * dt; p.y += p.vy * dt; p.z += p.vz * dt;
            p.vy -= 10.0f * dt;
            p.life -= dt * 0.2f;

            // Only remove when particle falls back to ground level
            if (p.y < -7.5f) {
                parts[i] = parts[--numParts];
            } else {
                float brightness = fmaxf(p.life, 0.15f); // keep visible even when life is low
                Color c = Color(p.hue, RainbowGradientPalette);
                c.scale((uint8_t)(brightness * 255));
                voxel(Vector3(p.x, p.y, p.z), c);
                i++;
            }
        }
    }
};

//=============================================================================
// Demo 35: Explosion Loop
//=============================================================================
class ExplosionDemo : public DemoAnimation {
    static const int NUM_P = 250;
    struct EPart { Vector3 dir; float dist; uint8_t hue; };
    EPart parts[NUM_P];
    float phase = 0;

public:
    void init() override {
        phase = 0;
        for (int i = 0; i < NUM_P; i++) {
            parts[i].dir = Vector3(noise.nextRandom(-1,1), noise.nextRandom(-1,1), noise.nextRandom(-1,1)).normalize();
            parts[i].dist = noise.nextRandom(0.5f, 1.0f);
            parts[i].hue = (uint8_t)(rand() % 256);
        }
    }

    const char* name() override { return "Explosion Loop"; }

    void update(float dt) override {
        phase += dt * 0.8f;
        float cycle = fmodf(phase, 3.0f);
        float expand = (cycle < 1.5f) ? cycle / 1.5f : 1.0f - (cycle - 1.5f) / 1.5f;
        expand = expand * expand;

        for (int i = 0; i < NUM_P; i++) {
            Vector3 p = parts[i].dir * parts[i].dist * expand * 7.5f;
            float brightness = (cycle < 1.5f) ? 1.0f : (1.0f - (cycle - 1.5f) / 1.5f);
            Color c = Color(parts[i].hue, LavaPalette);
            c.scale((uint8_t)(brightness * 255));
            radiate(p, c, 1.5f);
        }
    }
};

//=============================================================================
// Demo 36: Boids (Flocking)
//=============================================================================
class BoidsDemo : public DemoAnimation {
    static const int NUM_BOIDS = 50;
    struct Boid { float x,y,z,vx,vy,vz; uint8_t hue; };
    Boid boids[NUM_BOIDS];

public:
    void init() override {
        for (int i = 0; i < NUM_BOIDS; i++) {
            boids[i].x = noise.nextRandom(-5,5);
            boids[i].y = noise.nextRandom(-5,5);
            boids[i].z = noise.nextRandom(-5,5);
            boids[i].vx = noise.nextRandom(-1,1);
            boids[i].vy = noise.nextRandom(-1,1);
            boids[i].vz = noise.nextRandom(-1,1);
            boids[i].hue = (uint8_t)(i * 5);
        }
    }

    const char* name() override { return "Boids (Flocking)"; }

    void update(float dt) override {
        for (int i = 0; i < NUM_BOIDS; i++) {
            float cx=0,cy=0,cz=0; // cohesion
            float sx=0,sy=0,sz=0; // separation
            float ax=0,ay=0,az=0; // alignment
            int neighbors = 0;

            for (int j = 0; j < NUM_BOIDS; j++) {
                if (i == j) continue;
                float dx=boids[j].x-boids[i].x, dy=boids[j].y-boids[i].y, dz=boids[j].z-boids[i].z;
                float dist = sqrtf(dx*dx+dy*dy+dz*dz);
                if (dist < 5.0f) {
                    cx += boids[j].x; cy += boids[j].y; cz += boids[j].z;
                    ax += boids[j].vx; ay += boids[j].vy; az += boids[j].vz;
                    if (dist < 2.0f) { sx -= dx/dist; sy -= dy/dist; sz -= dz/dist; }
                    neighbors++;
                }
            }

            if (neighbors > 0) {
                cx = cx/neighbors - boids[i].x; cy = cy/neighbors - boids[i].y; cz = cz/neighbors - boids[i].z;
                ax = ax/neighbors; ay = ay/neighbors; az = az/neighbors;
                boids[i].vx += (cx*0.01f + sx*0.05f + ax*0.05f);
                boids[i].vy += (cy*0.01f + sy*0.05f + ay*0.05f);
                boids[i].vz += (cz*0.01f + sz*0.05f + az*0.05f);
            }

            // Wall avoidance
            if (boids[i].x < -6) boids[i].vx += 0.5f; if (boids[i].x > 6) boids[i].vx -= 0.5f;
            if (boids[i].y < -6) boids[i].vy += 0.5f; if (boids[i].y > 6) boids[i].vy -= 0.5f;
            if (boids[i].z < -6) boids[i].vz += 0.5f; if (boids[i].z > 6) boids[i].vz -= 0.5f;

            // Speed limit
            float spd = sqrtf(boids[i].vx*boids[i].vx+boids[i].vy*boids[i].vy+boids[i].vz*boids[i].vz);
            if (spd > 4.0f) { boids[i].vx*=4/spd; boids[i].vy*=4/spd; boids[i].vz*=4/spd; }

            boids[i].x += boids[i].vx*dt; boids[i].y += boids[i].vy*dt; boids[i].z += boids[i].vz*dt;

            Color c = Color(boids[i].hue, RainbowGradientPalette);
            radiate5(Vector3(boids[i].x, boids[i].y, boids[i].z), c, 2.0f);
        }
    }
};

//=============================================================================
// Demo 37: Matrix Rain
//=============================================================================
class MatrixDemo : public DemoAnimation {
    struct Column { float y; float speed; uint8_t length; };
    Column cols[16][16];

public:
    void init() override {
        for (int x = 0; x < 16; x++)
            for (int z = 0; z < 16; z++) {
                cols[x][z].y = noise.nextRandom(0, 20);
                cols[x][z].speed = noise.nextRandom(5, 15);
                cols[x][z].length = 4 + rand() % 8;
            }
    }

    const char* name() override { return "Matrix Rain"; }

    void update(float dt) override {
        for (int x = 0; x < 16; x++) {
            for (int z = 0; z < 16; z++) {
                Column& col = cols[x][z];
                col.y -= col.speed * dt;
                if (col.y < -(int)col.length) {
                    col.y = 16 + noise.nextRandom(0, 8);
                    col.speed = noise.nextRandom(5, 15);
                    col.length = 4 + rand() % 8;
                }

                for (int i = 0; i < col.length; i++) {
                    int y = (int)(col.y + i);
                    if (y >= 0 && y < 16) {
                        float brightness = 1.0f - (float)i / col.length;
                        if (i == 0) {
                            voxel((uint8_t)x, (uint8_t)y, (uint8_t)z, Color(200, 255, 200));
                        } else {
                            voxel((uint8_t)x, (uint8_t)y, (uint8_t)z,
                                  Color(0, (uint8_t)(255 * brightness), 0));
                        }
                    }
                }
            }
        }
    }
};

//=============================================================================
// Demo 38: Ripple
//=============================================================================
class RippleDemo : public DemoAnimation {
    struct Source { float x, y, z, time; };
    static const int MAX_SRC = 5;
    Source sources[MAX_SRC];
    float timer = 0;
    int nextSrc = 0;
    uint16_t hue16 = 0;

public:
    void init() override {
        timer = 0; nextSrc = 0; hue16 = 0;
        for (int i = 0; i < MAX_SRC; i++) sources[i].time = -100;
    }

    const char* name() override { return "Ripple"; }

    void update(float dt) override {
        timer += dt;
        hue16 += (uint16_t)(dt * 40 * 255);

        if (timer > 1.5f) {
            timer = 0;
            Source& s = sources[nextSrc++ % MAX_SRC];
            s.x = noise.nextRandom(-5, 5);
            s.y = noise.nextRandom(-5, 5);
            s.z = noise.nextRandom(-5, 5);
            s.time = 0;
        }

        for (int i = 0; i < MAX_SRC; i++) {
            if (sources[i].time < 0) continue;
            sources[i].time += dt;
        }

        for (int x = 0; x < 16; x++) {
            for (int y = 0; y < 16; y++) {
                for (int z = 0; z < 16; z++) {
                    float fx=x-7.5f, fy=y-7.5f, fz=z-7.5f;
                    float val = 0;
                    for (int i = 0; i < MAX_SRC; i++) {
                        if (sources[i].time < 0 || sources[i].time > 4) continue;
                        float dx=fx-sources[i].x, dy=fy-sources[i].y, dz=fz-sources[i].z;
                        float dist = sqrtf(dx*dx+dy*dy+dz*dz);
                        float wave = sinf(dist * 2 - sources[i].time * 8);
                        float fade = fmaxf(0.0f, 1.0f - sources[i].time * 0.25f);
                        float shell = fmaxf(0.0f, 1.0f - fabsf(dist - sources[i].time * 4) * 0.5f);
                        val += wave * fade * shell;
                    }
                    if (fabsf(val) > 0.2f) {
                        Color c = Color((uint8_t)((hue16 >> 8) + (int)(val * 40)), RainbowGradientPalette);
                        c.scale((uint8_t)(fminf(fabsf(val), 1.0f) * 255));
                        voxel((uint8_t)x, (uint8_t)y, (uint8_t)z, c);
                    }
                }
            }
        }
    }
};

//=============================================================================
// Demo 39: Kaleidoscope
//=============================================================================
class KaleidoscopeDemo : public DemoAnimation {
    float phase = 0;

public:
    void init() override { phase = 0; }
    const char* name() override { return "Kaleidoscope"; }

    void update(float dt) override {
        phase += dt * 0.5f;
        for (int x = 0; x < 8; x++) {
            for (int y = 0; y < 8; y++) {
                for (int z = 0; z < 8; z++) {
                    float n = noise.noise4(x * 0.3f, y * 0.3f, z * 0.3f, phase);
                    if (n > 0.35f) {
                        float brightness = (n - 0.35f) / 0.65f;
                        Color c = Color((uint8_t)(n * 255 + phase * 50), RainbowGradientPalette);
                        c.scale((uint8_t)(brightness * 255));

                        // Mirror across all 3 axes (8 copies)
                        uint8_t mx = 15 - x, my = 15 - y, mz = 15 - z;
                        voxel((uint8_t)x,  (uint8_t)y,  (uint8_t)z,  c);
                        voxel(mx,           (uint8_t)y,  (uint8_t)z,  c);
                        voxel((uint8_t)x,  my,           (uint8_t)z,  c);
                        voxel(mx,           my,           (uint8_t)z,  c);
                        voxel((uint8_t)x,  (uint8_t)y,  mz,           c);
                        voxel(mx,           (uint8_t)y,  mz,           c);
                        voxel((uint8_t)x,  my,           mz,           c);
                        voxel(mx,           my,           mz,           c);
                    }
                }
            }
        }
    }
};

//=============================================================================
// Demo 40: Wave Interference
//=============================================================================
class InterferenceDemo : public DemoAnimation {
    float phase = 0;
    uint16_t hue16 = 0;

public:
    void init() override { phase = 0; hue16 = 0; }
    const char* name() override { return "Wave Interference"; }

    void update(float dt) override {
        phase += dt;
        hue16 += (uint16_t)(dt * 20 * 255);

        // 4 fixed wave sources at cube corners
        float sx[4] = {-7, 7, -7, 7};
        float sy[4] = {-7, -7, 7, 7};
        float sz[4] = {-7, 7, 7, -7};
        float freq[4] = {3, 3.5f, 4, 2.5f};

        for (int x = 0; x < 16; x++) {
            for (int y = 0; y < 16; y++) {
                for (int z = 0; z < 16; z++) {
                    float fx=x-7.5f, fy=y-7.5f, fz=z-7.5f;
                    float val = 0;
                    for (int i = 0; i < 4; i++) {
                        float dx=fx-sx[i], dy=fy-sy[i], dz=fz-sz[i];
                        float dist = sqrtf(dx*dx+dy*dy+dz*dz);
                        val += sinf(dist * freq[i] * 0.5f - phase * 4);
                    }
                    val /= 4;

                    if (fabsf(val) > 0.3f) {
                        float brightness = (fabsf(val) - 0.3f) / 0.7f;
                        Color c = Color((uint8_t)((hue16 >> 8) + (int)(val * 60)), RainbowGradientPalette);
                        c.scale((uint8_t)(brightness * 255));
                        voxel((uint8_t)x, (uint8_t)y, (uint8_t)z, c);
                    }
                }
            }
        }
    }
};

//=============================================================================
// Main
//=============================================================================
int main() {
    if (!Renderer::init(1280, 720)) {
        return 1;
    }

    Display::begin();

    // Create demo animations
    DemoAnimation* demos[] = {
        new PlasmaDemo(),
        new CubeDemo(),
        new AtomsDemo(),
        new SinusDemo(),
        new StarfieldDemo(),
        new HelixDemo(),
        new FireworksDemo(),
        new LifeDemo(),
        new TwinkelsDemo(),
        new ArrowsDemo(),
        new MarioDemo(),
        new ScrollerDemo(),
        // New patterns
        new RainDemo(),
        new AuroraDemo(),
        new LavaLampDemo(),
        new FirefliesDemo(),
        new OceanDemo(),
        new LorenzDemo(),
        new TorusDemo(),
        new SierpinskiDemo(),
        new MoebiusDemo(),
        new SpirographDemo(),
        new SnakeDemo(),
        new TetrisDemo(),
        new PongDemo(),
        new MazeDemo(),
        new GlobeDemo(),
        new DNADemo(),
        new ClockDemo(),
        new HeartDemo(),
        new HourglassDemo(),
        new GalaxyDemo(),
        new TornadoDemo(),
        new FountainDemo(),
        new ExplosionDemo(),
        new BoidsDemo(),
        new MatrixDemo(),
        new RippleDemo(),
        new KaleidoscopeDemo(),
        new InterferenceDemo()
    };
    int numDemos = sizeof(demos) / sizeof(demos[0]);
    int currentDemo = 0;

    demos[currentDemo]->init();
    printf("Animation: %s\n", demos[currentDemo]->name());
    printf("Press RIGHT/SPACE for next, LEFT for previous, R to reset\n\n");

    while (!Renderer::shouldClose()) {
        float dt = Renderer::getDeltaTime();

        Renderer::beginFrame();

        if (Renderer::wasKeyPressed(GLFW_KEY_SPACE) || Renderer::wasKeyPressed(GLFW_KEY_RIGHT)) {
            currentDemo = (currentDemo + 1) % numDemos;
            demos[currentDemo]->init();
            printf("Animation: %s\n", demos[currentDemo]->name());
        }

        if (Renderer::wasKeyPressed(GLFW_KEY_LEFT)) {
            currentDemo = (currentDemo - 1 + numDemos) % numDemos;
            demos[currentDemo]->init();
            printf("Animation: %s\n", demos[currentDemo]->name());
        }

        if (Renderer::wasKeyPressed(GLFW_KEY_R)) {
            demos[currentDemo]->init();
            printf("Reset: %s\n", demos[currentDemo]->name());
        }

        demos[currentDemo]->update(dt);

        Display::update();
        Renderer::renderCube((uint8_t(*)[16][16][3])Display::getRawBuffer(), simConfig.power.brightness);

        Renderer::endFrame();
    }

    for (int i = 0; i < numDemos; i++) {
        delete demos[i];
    }

    Renderer::shutdown();
    return 0;
}
