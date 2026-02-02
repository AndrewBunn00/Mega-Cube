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
        new ScrollerDemo()
    };
    int numDemos = sizeof(demos) / sizeof(demos[0]);
    int currentDemo = 0;

    demos[currentDemo]->init();
    printf("Animation: %s\n", demos[currentDemo]->name());
    printf("Press SPACE for next animation, R to reset\n\n");

    while (!Renderer::shouldClose()) {
        float dt = Renderer::getDeltaTime();

        Renderer::beginFrame();

        if (Renderer::wasKeyPressed(GLFW_KEY_SPACE)) {
            currentDemo = (currentDemo + 1) % numDemos;
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
