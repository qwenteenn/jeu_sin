#include "Enemy.h"
#include "Player.h"

Enemy::Enemy(EnemyType t, Vector3 pos, int floor, bool ally) : type(t), position(pos), allied(ally), floorLevel(floor) {
    const float scale = 1.0f + (floor - 1) * 0.35f;
    switch (type) {
        case EnemyType::Scarab:
            maxHp = 22 * scale; damage = 7 * scale; speed = 4.8f; radius = 0.45f; attackRange = 1.1f; break;
        case EnemyType::Skeleton:
            maxHp = 38 * scale; damage = 10 * scale; speed = 3.4f; radius = 0.55f; attackRange = 1.35f; break;
        case EnemyType::Mummy:
            maxHp = 70 * scale; damage = 13 * scale; speed = 2.0f; radius = 0.65f; attackRange = 1.35f; break;
        case EnemyType::Specter:
            maxHp = 34 * scale; damage = 9 * scale; speed = 3.1f; radius = 0.5f; attackRange = 8.0f; break;
        case EnemyType::Golem:
            maxHp = 130 * scale; damage = 23 * scale; speed = 1.65f; radius = 0.9f; attackRange = 1.8f; break;
        case EnemyType::SummonedSkeleton:
            maxHp = 35 * scale; damage = 8 * scale; speed = 4.0f; radius = 0.5f; attackRange = 1.25f; allied = true; lifeTimer = 18.0f; break;
    }
    hp = maxHp;
}

bool Enemy::IsRanged() const {
    return type == EnemyType::Specter;
}

void Enemy::Update(float dt, Player& player, std::vector<Projectile>& projectiles, std::vector<Enemy>& enemies) {
    if (!alive) return;
    if (lifeTimer > 0.0f) {
        lifeTimer -= dt;
        if (lifeTimer <= 0.0f) alive = false;
    }
    attackCooldown = std::max(0.0f, attackCooldown - dt);

    if (allied) {
        Enemy* target = nullptr;
        float best = 9999.0f;
        for (auto& e : enemies) {
            if (!e.alive || e.allied) continue;
            const float d = DistanceXZ(position, e.position);
            if (d < best) { best = d; target = &e; }
        }
        if (!target) return;
        Vector3 dir = DirectionXZ(position, target->position);
        if (best > attackRange) position = AddScaled(position, dir, speed * dt);
        else if (attackCooldown <= 0.0f) {
            target->TakeDamage(damage);
            attackCooldown = 0.8f;
        }
        return;
    }

    const float d = DistanceXZ(position, player.position);
    if (d > detectionRange) return;
    Vector3 dir = DirectionXZ(position, player.position);

    if (IsRanged()) {
        if (d > 6.0f) position = AddScaled(position, dir, speed * dt);
        if (attackCooldown <= 0.0f && d < 12.0f) {
            Projectile p;
            p.owner = ProjectileOwner::Enemy;
            p.damage = damage;
            p.damageType = DamageType::Dark;
            p.radius = 0.22f;
            p.position = {position.x, 1.0f, position.z};
            p.velocity = {dir.x * 9.0f, 0.0f, dir.z * 9.0f};
            p.lifetime = 2.2f;
            p.color = VIOLET;
            projectiles.push_back(p);
            attackCooldown = 1.25f;
        }
    } else {
        if (d > attackRange) position = AddScaled(position, dir, speed * dt);
        else if (attackCooldown <= 0.0f) {
            player.TakeDamage(damage, DamageType::Physical, UpgradeData{});
            attackCooldown = 0.9f + RandomFloat(0.0f, 0.35f);
        }
    }
}

void Enemy::TakeDamage(float amount) {
    hp -= amount;
    if (hp <= 0.0f) {
        hp = 0.0f;
        alive = false;
    }
}

std::vector<LootDrop> Enemy::GenerateLoot(float lootBonus) const {
    std::vector<LootDrop> drops;
    if (allied) return drops;
    const auto add = [&](const std::string& id, int amount, float ox, float oz) {
        LootDrop drop;
        drop.itemId = id;
        drop.amount = amount;
        drop.position = {position.x + ox, 0.0f, position.z + oz};
        drops.push_back(drop);
    };
    const float bonus = 1.0f + lootBonus;
    int goldAmount = 0;
    switch (type) {
        case EnemyType::Scarab:
            goldAmount = RandomInt(4, 10) * floorLevel;
            add("scarab_shell", 1, -0.25f, 0.2f);
            if (RandomFloat(0, 1) < 0.20f * bonus) add("weak_venom", 1, 0.35f, -0.1f);
            break;
        case EnemyType::Skeleton:
            goldAmount = RandomInt(7, 15) * floorLevel;
            add("ancient_bone", 1, -0.25f, 0.2f);
            if (RandomFloat(0, 1) < 0.18f * bonus) add("rusted_weapon_fragment", 1, 0.35f, -0.1f);
            break;
        case EnemyType::Mummy:
            goldAmount = RandomInt(12, 24) * floorLevel;
            add("cursed_bandage", 1, -0.25f, 0.2f);
            if (RandomFloat(0, 1) < 0.35f * bonus) add("ancient_dust", 1, 0.35f, -0.1f);
            break;
        case EnemyType::Specter:
            goldAmount = RandomInt(10, 20) * floorLevel;
            add("spectral_essence", 1, -0.25f, 0.2f);
            if (RandomFloat(0, 1) < 0.25f * bonus) add("soul_fragment", 1, 0.35f, -0.1f);
            if (RandomFloat(0, 1) < 0.22f * bonus) add("weak_crystal", 1, 0.0f, 0.5f);
            break;
        case EnemyType::Golem:
            goldAmount = RandomInt(20, 36) * floorLevel;
            add("golem_fragment", 1, -0.25f, 0.2f);
            if (RandomFloat(0, 1) < 0.18f * bonus) add("stone_core", 1, 0.35f, -0.1f);
            break;
        default:
            break;
    }
    if (goldAmount > 0) add("gold", goldAmount, 0.25f, 0.25f);
    if (RandomFloat(0, 1) < 0.07f * bonus) add("broken_relic", 1, -0.4f, -0.4f);
    if (RandomFloat(0, 1) < 0.04f * bonus) add("return_stone", 1, 0.4f, 0.4f);
    return drops;
}

void Enemy::Draw() const {
    if (!alive) return;
    Color c = RED;
    switch (type) {
        case EnemyType::Scarab: c = DARKGREEN; break;
        case EnemyType::Skeleton: c = RAYWHITE; break;
        case EnemyType::Mummy: c = BEIGE; break;
        case EnemyType::Specter: c = VIOLET; break;
        case EnemyType::Golem: c = GRAY; break;
        case EnemyType::SummonedSkeleton: c = SKYBLUE; break;
    }
    Vector3 pos = {position.x, radius, position.z};
    if (type == EnemyType::Scarab) DrawSphere(pos, radius, c);
    else if (type == EnemyType::Specter) DrawSphere({pos.x, pos.y + 0.35f, pos.z}, radius, FadeColor(c, 0.8f));
    else DrawCube(pos, radius * 1.6f, radius * 2.0f, radius * 1.6f, c);
    DrawCircle3D({position.x, 0.05f, position.z}, radius * 1.3f, {1, 0, 0}, 90, allied ? SKYBLUE : RED);
    const float pct = maxHp > 0.0f ? hp / maxHp : 0.0f;
    DrawCube({position.x, 2.1f, position.z}, 1.3f, 0.08f, 0.08f, DARKGRAY);
    DrawCube({position.x - 0.65f + 0.65f * pct, 2.11f, position.z}, 1.3f * pct, 0.09f, 0.09f, allied ? SKYBLUE : GREEN);
}
