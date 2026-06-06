#pragma once
#include "Common.h"
#include "Enemy.h"
#include "Loot.h"
#include "Room.h"
#include "Types.h"

class Dungeon {
public:
    int currentFloor = 1;
    int maxUnlockedFloor = 1;
    std::vector<Room> rooms;
    std::vector<Enemy> enemies;
    std::vector<LootDrop> loot;
    bool bossSpawned = false;
    bool bossDefeated = false;

    void GenerateFloor(int floor);
    Room* CurrentRoom(Vector3 playerPos);
    const Room* CurrentRoom(Vector3 playerPos) const;
    void EnterRoom(Room& room);
    bool AllHostilesClearedInRoom(const Room& room) const;
    void UpdateRoomStates(Vector3 playerPos);
    Vector3 StartPositionForFloor(int floor) const;
    void ResetRun();
    void Draw(Vector3 playerPos) const;

private:
    void AddStandardSpawns(Room& room, int difficulty);
};
