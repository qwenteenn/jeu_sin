#include "Trap.h"
#include "Player.h"
#include "UpgradeStation.h"

Trap::Trap(TrapType t, Vector3 pos, Vector3 s, float dmg) : type(t), position(pos), size(s), damage(dmg) {
    timer = RandomFloat(0.0f, interval);
    if (type == TrapType::FireJet) { interval = 2.8f; activeTime = 1.1f; }
    if (type == TrapType::ArrowWall) { interval = 2.2f; activeTime = 0.2f; }
    if (type == TrapType::Quicksand) { interval = 1.0f; activeTime = 1.0f; }
    if (type == TrapType::CrumblingFloor) { interval = 4.0f; activeTime = 1.0f; }
}

bool Trap::IsActive() const {
    if (spent) return false;
    if (type == TrapType::Quicksand) return true;
    return std::fmod(timer, interval) < activeTime;
}

void Trap::Update(float dt, Player& player, const UpgradeData& upgrades) {
    if (spent) return;
    timer += dt;
    const bool inside = PointInsideXZ(player.position, position, size);
    if (!inside) return;

    switch (type) {
        case TrapType::Spikes:
            if (IsActive()) player.TakeDamage(damage, DamageType::Trap, upgrades);
            break;
        case TrapType::FireJet:
            if (IsActive()) player.TakeDamage(damage * dt * 3.0f, DamageType::Trap, upgrades);
            break;
        case TrapType::ArrowWall:
            if (IsActive()) player.TakeDamage(damage, DamageType::Trap, upgrades);
            break;
        case TrapType::Quicksand:
            player.position.x += (position.x - player.position.x) * dt * 0.25f;
            player.position.z += (position.z - player.position.z) * dt * 0.25f;
            if (RandomFloat(0, 1) < 0.025f) player.TakeDamage(damage * 0.35f, DamageType::Trap, upgrades);
            break;
        case TrapType::CrumblingFloor:
            if (!triggered) { triggered = true; timer = 0.0f; }
            if (triggered && timer > 1.2f) {
                player.TakeDamage(damage * 1.5f, DamageType::Trap, upgrades);
                spent = true;
            }
            break;
        case TrapType::ChestBomb:
            break;
    }
}

void Trap::Draw() const {
    if (spent) return;
    Color c = DARKGRAY;
    float height = 0.08f;
    if (type == TrapType::Spikes) { c = IsActive() ? RED : MAROON; height = IsActive() ? 0.7f : 0.1f; }
    if (type == TrapType::FireJet) { c = IsActive() ? ORANGE : BROWN; height = IsActive() ? 1.4f : 0.1f; }
    if (type == TrapType::ArrowWall) { c = IsActive() ? YELLOW : DARKGRAY; height = 0.25f; }
    if (type == TrapType::Quicksand) { c = Color{194, 178, 128, 255}; height = 0.04f; }
    if (type == TrapType::CrumblingFloor) { c = triggered ? RED : GRAY; height = 0.05f; }

    DrawCube({position.x, height * 0.5f, position.z}, size.x * 2.0f, height, size.z * 2.0f, c);
    DrawCubeWires({position.x, height * 0.5f + 0.01f, position.z}, size.x * 2.0f, height + 0.02f, size.z * 2.0f, BLACK);
}
