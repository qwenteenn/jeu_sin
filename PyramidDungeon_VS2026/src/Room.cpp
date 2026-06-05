#include "Room.h"

bool Room::Contains(Vector3 p) const {
    return PointInsideXZ(p, center, halfSize);
}

std::string Room::Name() const {
    switch (type) {
        case RoomType::Entrance: return "Entree";
        case RoomType::Combat: return "Salle de combat";
        case RoomType::Trap: return "Salle de pieges";
        case RoomType::Chest: return "Salle de coffre";
        case RoomType::Rest: return "Autel de repos";
        case RoomType::Cursed: return "Salle maudite";
        case RoomType::MiniBoss: return "Mini-boss";
        case RoomType::Transition: return "Cercle de descente";
        case RoomType::Boss: return "Salle du boss";
    }
    return "Salle";
}

void Room::Draw(bool current) const {
    Color floorColor = current ? Color{58, 51, 58, 255} : Color{42, 38, 42, 255};
    if (type == RoomType::Rest) floorColor = Color{36, 48, 55, 255};
    if (type == RoomType::Cursed) floorColor = Color{58, 35, 58, 255};
    if (type == RoomType::Boss) floorColor = Color{65, 42, 36, 255};

    DrawCube({center.x, -0.05f, center.z}, halfSize.x * 2.0f, 0.1f, halfSize.z * 2.0f, floorColor);
    DrawCubeWires({center.x, 0.02f, center.z}, halfSize.x * 2.0f, 0.1f, halfSize.z * 2.0f, DARKGRAY);

    const float wallH = 3.2f;
    DrawCube({center.x, wallH * 0.5f, center.z - halfSize.z}, halfSize.x * 2.0f, wallH, 0.35f, Color{70, 64, 58, 255});
    DrawCube({center.x, wallH * 0.5f, center.z + halfSize.z}, halfSize.x * 2.0f, wallH, 0.35f, Color{70, 64, 58, 255});
    DrawCube({center.x - halfSize.x, wallH * 0.5f, center.z}, 0.35f, wallH, halfSize.z * 2.0f, Color{70, 64, 58, 255});
    DrawCube({center.x + halfSize.x, wallH * 0.5f, center.z}, 0.35f, wallH, halfSize.z * 2.0f, Color{70, 64, 58, 255});

    for (int i = -1; i <= 1; i += 2) {
        for (int j = -1; j <= 1; j += 2) {
            DrawCylinder({center.x + i * (halfSize.x - 1.2f), 0, center.z + j * (halfSize.z - 1.2f)}, 0.42f, 0.55f, 2.6f, 8, Color{91, 82, 72, 255});
            DrawSphere({center.x + i * (halfSize.x - 1.2f), 2.85f, center.z + j * (halfSize.z - 1.2f)}, 0.28f, current ? GOLD : ORANGE);
        }
    }

    for (const auto& trap : traps) trap.Draw();

    if (type == RoomType::Chest && !chestOpened) {
        DrawCube({center.x, 0.45f, center.z}, 1.2f, 0.9f, 0.9f, BROWN);
        DrawCubeWires({center.x, 0.46f, center.z}, 1.25f, 0.95f, 0.95f, GOLD);
    }

    if (type == RoomType::Transition) {
        Color c = transitionUnlocked ? SKYBLUE : DARKBLUE;
        DrawCylinder({center.x, 0.03f, center.z}, 2.4f, 2.4f, 0.08f, 48, FadeColor(c, 0.85f));
        DrawCylinderWires({center.x, 0.08f, center.z}, 2.55f, 2.55f, 0.1f, 48, WHITE);
    }

    if (type == RoomType::Rest && !rewardTaken) {
        DrawCylinder({center.x, 0, center.z}, 1.2f, 1.0f, 1.0f, 16, SKYBLUE);
        DrawSphere({center.x, 1.45f, center.z}, 0.45f, FadeColor(SKYBLUE, 0.85f));
    }
}
