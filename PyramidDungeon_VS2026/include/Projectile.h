#pragma once
#include "Common.h"
#include "Types.h"

struct Projectile {
    Vector3 position {0, 0, 0};
    Vector3 velocity {0, 0, 0};
    float damage = 0.0f;
    float radius = 0.25f;
    float lifetime = 2.0f;
    ProjectileOwner owner = ProjectileOwner::Player;
    DamageType damageType = DamageType::Physical;
    Color color = WHITE;
    bool alive = true;

    void Update(float dt);
    void Draw() const;
};
