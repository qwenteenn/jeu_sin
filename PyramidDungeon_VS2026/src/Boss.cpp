#include "Boss.h"
#include "Player.h"

void Boss::Spawn(Vector3 pos, int floor) {
    position = pos;
    maxHp = 850.0f + floor * 150.0f;
    hp = maxHp;
    radius = 1.8f;
    speed = 1.6f;
    attackCooldown = 1.5f;
    shockwaveRadius = 0.0f;
    shockwaveActive = false;
    alive = true;
    phaseTwo = false;
}

void Boss::Update(float dt, Player& player, std::vector<Enemy>& enemies, std::vector<Projectile>& projectiles) {
    if (!alive) return;
    if (hp < maxHp * 0.5f) phaseTwo = true;
    const float bossSpeed = phaseTwo ? speed * 1.45f : speed;
    const float dist = DistanceXZ(position, player.position);
    Vector3 dir = DirectionXZ(position, player.position);

    if (dist > 3.2f) position = AddScaled(position, dir, bossSpeed * dt);

    attackCooldown -= dt;
    if (attackCooldown <= 0.0f) {
        const int choice = RandomInt(0, phaseTwo ? 3 : 2);
        if (choice == 0) {
            shockwaveActive = true;
            shockwaveRadius = 0.2f;
            attackCooldown = phaseTwo ? 2.3f : 3.0f;
        } else if (choice == 1) {
            position = AddScaled(position, dir, 6.5f);
            if (DistanceXZ(position, player.position) < 3.0f) player.TakeDamage(36.0f, DamageType::Physical, UpgradeData{});
            attackCooldown = phaseTwo ? 1.8f : 2.4f;
        } else if (choice == 2) {
            enemies.emplace_back(EnemyType::Scarab, Vector3{position.x + 2, 0, position.z + 2}, 3);
            enemies.emplace_back(EnemyType::Skeleton, Vector3{position.x - 2, 0, position.z + 2}, 3);
            if (phaseTwo) enemies.emplace_back(EnemyType::Specter, Vector3{position.x, 0, position.z - 3}, 3);
            attackCooldown = phaseTwo ? 2.6f : 3.4f;
        } else {
            Projectile p;
            p.owner = ProjectileOwner::Enemy;
            p.damage = 42.0f;
            p.damageType = DamageType::Magical;
            p.radius = 0.38f;
            p.position = {position.x, 1.2f, position.z};
            p.velocity = {dir.x * 14.0f, 0, dir.z * 14.0f};
            p.lifetime = 1.9f;
            p.color = GOLD;
            projectiles.push_back(p);
            attackCooldown = 2.1f;
        }
    }

    if (shockwaveActive) {
        shockwaveRadius += dt * (phaseTwo ? 10.0f : 8.0f);
        if (std::abs(DistanceXZ(position, player.position) - shockwaveRadius) < 0.45f) {
            player.TakeDamage(shockwaveDamage * (phaseTwo ? 1.3f : 1.0f), DamageType::Magical, UpgradeData{});
        }
        if (shockwaveRadius > 10.0f) shockwaveActive = false;
    }
}

void Boss::TakeDamage(float amount) {
    if (!alive) return;
    hp -= amount;
    if (hp <= 0.0f) {
        hp = 0.0f;
        alive = false;
    }
}

std::vector<LootDrop> Boss::GenerateLoot() const {
    std::vector<LootDrop> drops;
    auto add = [&](const std::string& id, int amount, float ox, float oz) {
        LootDrop d;
        d.itemId = id;
        d.amount = amount;
        d.position = {position.x + ox, 0, position.z + oz};
        drops.push_back(d);
    };
    add("gold", 550, -1.5f, 0.0f);
    add("ancient_relic", 2, 0.0f, 0.0f);
    add("stable_return_crystal", 1, 1.5f, 0.0f);
    add("boss_guardian_core", 1, 0.0f, 1.5f);
    if (RandomFloat(0, 1) < 0.30f) add("rare_guardian_blade", 1, 0.0f, -1.5f);
    return drops;
}

void Boss::Draw() const {
    if (!alive) return;
    Color body = phaseTwo ? RED : Color{112, 87, 66, 255};
    DrawCube({position.x, 1.9f, position.z}, 3.1f, 3.8f, 3.1f, body);
    DrawCubeWires({position.x, 1.9f, position.z}, 3.2f, 3.9f, 3.2f, GOLD);
    DrawSphere({position.x, 4.1f, position.z}, 0.9f, phaseTwo ? ORANGE : GOLD);
    DrawCylinder({position.x, 0.03f, position.z}, radius, radius, 0.08f, 32, FadeColor(RED, 0.25f));
    if (shockwaveActive) {
        DrawCylinderWires({position.x, 0.06f, position.z}, shockwaveRadius, shockwaveRadius, 0.1f, 64, GOLD);
    }
}
