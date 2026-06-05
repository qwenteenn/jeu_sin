#include "Dungeon.h"

void Dungeon::GenerateFloor(int floor) {
    currentFloor = floor;
    rooms.clear();
    enemies.clear();
    loot.clear();
    bossSpawned = false;
    bossDefeated = false;

    const int roomCount = floor == 3 ? 8 : 7;
    const float spacing = 20.0f;
    for (int i = 0; i < roomCount; ++i) {
        Room r;
        r.id = i;
        r.floor = floor;
        r.center = {i * spacing, 0.0f, floor * 35.0f};
        r.halfSize = {8.5f, 2.0f, 8.5f};
        r.type = RoomType::Combat;

        if (i == 0) r.type = RoomType::Entrance;
        else if (i == roomCount - 1 && floor < 3) r.type = RoomType::Transition;
        else if (i == roomCount - 1 && floor == 3) r.type = RoomType::Boss;
        else if (i == 1) r.type = RoomType::Combat;
        else if (i == 2) r.type = RoomType::Trap;
        else if (i == 3) r.type = RoomType::Chest;
        else if (i == 4) r.type = RoomType::Cursed;
        else if (i == 5) r.type = RoomType::Rest;
        else if (i == 6) r.type = RoomType::MiniBoss;

        if (r.type == RoomType::Combat || r.type == RoomType::Cursed || r.type == RoomType::MiniBoss) {
            AddStandardSpawns(r, floor + i / 2);
        }
        if (r.type == RoomType::Trap || r.type == RoomType::Cursed) {
            r.traps.push_back(Trap(TrapType::Spikes, {r.center.x - 3.0f, 0, r.center.z}, {1.3f, 0.1f, 3.0f}, 15.0f + floor * 3.0f));
            r.traps.push_back(Trap(TrapType::FireJet, {r.center.x + 2.8f, 0, r.center.z - 1.5f}, {1.0f, 0.1f, 2.3f}, 10.0f + floor * 2.0f));
            r.traps.push_back(Trap(TrapType::Quicksand, {r.center.x, 0, r.center.z + 3.2f}, {2.0f, 0.1f, 2.0f}, 8.0f + floor));
        }
        if (r.type == RoomType::MiniBoss) {
            r.spawns.clear();
            r.spawns.push_back({EnemyType::Golem, {r.center.x, 0, r.center.z}});
            r.spawns.push_back({EnemyType::Skeleton, {r.center.x - 3, 0, r.center.z + 2}});
            r.spawns.push_back({EnemyType::Skeleton, {r.center.x + 3, 0, r.center.z + 2}});
        }
        rooms.push_back(r);
    }
}

void Dungeon::AddStandardSpawns(Room& room, int difficulty) {
    const int count = std::min(6, 2 + difficulty);
    for (int i = 0; i < count; ++i) {
        EnemyType t = EnemyType::Scarab;
        const int roll = RandomInt(0, 100);
        if (difficulty >= 1) t = roll < 35 ? EnemyType::Scarab : EnemyType::Skeleton;
        if (difficulty >= 2 && roll > 55) t = EnemyType::Mummy;
        if (difficulty >= 3 && roll > 70) t = EnemyType::Specter;
        if (difficulty >= 4 && roll > 88) t = EnemyType::Golem;
        Vector3 p {room.center.x + RandomFloat(-5.0f, 5.0f), 0, room.center.z + RandomFloat(-5.0f, 5.0f)};
        room.spawns.push_back({t, p});
    }
}

Room* Dungeon::CurrentRoom(Vector3 playerPos) {
    for (auto& r : rooms) if (r.Contains(playerPos)) return &r;
    return nullptr;
}

const Room* Dungeon::CurrentRoom(Vector3 playerPos) const {
    for (const auto& r : rooms) if (r.Contains(playerPos)) return &r;
    return nullptr;
}

void Dungeon::EnterRoom(Room& room) {
    if (room.entered) return;
    room.entered = true;
    if (room.type == RoomType::Entrance || room.type == RoomType::Rest || room.type == RoomType::Chest || room.type == RoomType::Trap || room.type == RoomType::Transition) {
        room.cleared = room.spawns.empty();
    }
    for (const auto& spawn : room.spawns) {
        enemies.emplace_back(spawn.type, spawn.position, currentFloor);
    }
}

bool Dungeon::AllHostilesClearedInRoom(const Room& room) const {
    for (const auto& e : enemies) {
        if (!e.alive || e.allied) continue;
        if (PointInsideXZ(e.position, room.center, room.halfSize)) return false;
    }
    return true;
}

void Dungeon::UpdateRoomStates(Vector3 playerPos) {
    for (auto& room : rooms) {
        if (room.entered && !room.cleared && AllHostilesClearedInRoom(room)) {
            room.cleared = true;
            if (room.type == RoomType::Transition) room.transitionUnlocked = true;
        }
    }
    Room* current = CurrentRoom(playerPos);
    if (current) {
        EnterRoom(*current);
        if (current->type == RoomType::Transition) {
            bool prevCleared = true;
            for (const auto& r : rooms) {
                if (r.id < current->id && (r.type == RoomType::Combat || r.type == RoomType::Cursed || r.type == RoomType::MiniBoss) && !r.cleared) prevCleared = false;
            }
            current->transitionUnlocked = prevCleared && current->cleared;
        }
    }
}

Vector3 Dungeon::StartPositionForFloor(int floor) const {
    return {0.0f, 0.0f, floor * 35.0f};
}

void Dungeon::ResetRun() {
    GenerateFloor(currentFloor);
}

void Dungeon::Draw(Vector3 playerPos) const {
    const Room* current = CurrentRoom(playerPos);
    for (size_t i = 0; i < rooms.size(); ++i) {
        rooms[i].Draw(current && current->id == rooms[i].id);
        if (i + 1 < rooms.size()) {
            Vector3 a = rooms[i].center;
            Vector3 b = rooms[i + 1].center;
            Vector3 mid {(a.x + b.x) * 0.5f, -0.04f, (a.z + b.z) * 0.5f};
            DrawCube(mid, std::abs(b.x - a.x), 0.08f, 4.0f, Color{48, 44, 42, 255});
            DrawCubeWires(mid, std::abs(b.x - a.x), 0.09f, 4.0f, DARKGRAY);
        }
    }
    for (const auto& l : loot) if (l.alive) l.Draw();
    for (const auto& e : enemies) if (e.alive) e.Draw();
}
