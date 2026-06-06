#pragma once
#include "Common.h"

struct LootDrop {
    std::string itemId;
    int amount = 1;
    Vector3 position {0, 0, 0};
    float radius = 0.55f;
    float spin = 0.0f;
    bool alive = true;

    void Update(float dt);
    void Draw() const;
};
