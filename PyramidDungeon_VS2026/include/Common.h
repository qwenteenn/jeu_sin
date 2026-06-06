#pragma once

#include "raylib.h"
#include "raymath.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

constexpr int SCREEN_WIDTH = 1600;
constexpr int SCREEN_HEIGHT = 900;
constexpr float WORLD_Y = 0.0f;

inline float Clamp01(float v) {
    return std::max(0.0f, std::min(1.0f, v));
}

inline float DistanceXZ(Vector3 a, Vector3 b) {
    const float dx = a.x - b.x;
    const float dz = a.z - b.z;
    return std::sqrt(dx * dx + dz * dz);
}

inline Vector3 DirectionXZ(Vector3 from, Vector3 to) {
    Vector3 d { to.x - from.x, 0.0f, to.z - from.z };
    const float len = std::sqrt(d.x * d.x + d.z * d.z);
    if (len <= 0.0001f) return {0.0f, 0.0f, 0.0f};
    return {d.x / len, 0.0f, d.z / len};
}

inline Vector3 AddScaled(Vector3 a, Vector3 dir, float scale) {
    return { a.x + dir.x * scale, a.y + dir.y * scale, a.z + dir.z * scale };
}

inline bool PointInsideXZ(Vector3 p, Vector3 center, Vector3 half) {
    return p.x >= center.x - half.x && p.x <= center.x + half.x &&
           p.z >= center.z - half.z && p.z <= center.z + half.z;
}

inline float RandomFloat(float minValue, float maxValue) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(minValue, maxValue);
    return dist(rng);
}

inline int RandomInt(int minValue, int maxValue) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(minValue, maxValue);
    return dist(rng);
}

inline Color FadeColor(Color c, float alpha) {
    c.a = static_cast<unsigned char>(std::max(0, std::min(255, static_cast<int>(255.0f * alpha))));
    return c;
}
