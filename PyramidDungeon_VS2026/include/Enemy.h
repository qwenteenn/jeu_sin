#pragma once
#include "Common.h"
#include "Loot.h"
#include "Projectile.h"
#include "Types.h"

class Player;

class Enemy {
public:
    EnemyType type = EnemyType::Scarab;
    Vector3 position {0, 0, 0};
    float hp = 30.0f;
    float maxHp = 30.0f;
    float damage = 8.0f;
    float speed = 3.0f;
    float radius = 0.55f;
    float attackRange = 1.4f;
    float detectionRange = 18.0f;
    float attackCooldown = 0.0f;
    bool alive = true;
    bool allied = false;
    float lifeTimer = -1.0f;
    int floorLevel = 1;

    Enemy() = default;
    Enemy(EnemyType t, Vector3 pos, int floor, bool ally = false);
    void Update(float dt, Player& player, std::vector<Projectile>& projectiles, std::vector<Enemy>& enemies);
    void TakeDamage(float amount);
    std::vector<LootDrop> GenerateLoot(float lootBonus) const;
    void Draw() const;
    bool IsRanged() const;
};
