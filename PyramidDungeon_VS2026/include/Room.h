#pragma once
#include "Common.h"
#include "Trap.h"
#include "Types.h"

struct MobSpawn {
    EnemyType type = EnemyType::Scarab;
    Vector3 position {0, 0, 0};
};

struct Room {
    int id = 0;
    int floor = 1;
    RoomType type = RoomType::Combat;
    Vector3 center {0, 0, 0};
    Vector3 halfSize {8, 2, 8};
    std::vector<MobSpawn> spawns;
    std::vector<Trap> traps;
    bool entered = false;
    bool cleared = false;
    bool chestOpened = false;
    bool rewardTaken = false;
    bool transitionUnlocked = false;

    bool Contains(Vector3 p) const;
    void Draw(bool current) const;
    std::string Name() const;
};
