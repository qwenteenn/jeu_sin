#pragma once
#include "Common.h"
#include "Enemy.h"
#include "Loot.h"
#include "Projectile.h"
#include "Types.h"

class Player;

class Boss {
public:
    Vector3 position {0, 0, 0};
    float hp = 850.0f;
    float maxHp = 850.0f;
    float radius = 1.8f;
    float speed = 1.8f;
    float attackCooldown = 2.0f;
    float shockwaveRadius = 0.0f;
    float shockwaveDamage = 28.0f;
    bool shockwaveActive = false;
    bool alive = false;
    bool phaseTwo = false;

    void Spawn(Vector3 pos, int floor);
    void Update(float dt, Player& player, std::vector<Enemy>& enemies, std::vector<Projectile>& projectiles);
    void TakeDamage(float amount);
    std::vector<LootDrop> GenerateLoot() const;
    void Draw() const;
};
