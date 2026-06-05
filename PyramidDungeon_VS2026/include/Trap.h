#pragma once
#include "Common.h"
#include "Types.h"

class Player;
struct UpgradeData;

struct Trap {
    TrapType type = TrapType::Spikes;
    Vector3 position {0, 0, 0};
    Vector3 size {1, 1, 1};
    float timer = 0.0f;
    float interval = 2.0f;
    float activeTime = 0.8f;
    float damage = 15.0f;
    bool triggered = false;
    bool spent = false;

    Trap() = default;
    Trap(TrapType t, Vector3 pos, Vector3 s, float dmg);
    void Update(float dt, Player& player, const UpgradeData& upgrades);
    void Draw() const;
    bool IsActive() const;
};
