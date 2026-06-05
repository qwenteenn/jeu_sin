#include "Loot.h"
#include "ItemDatabase.h"

void LootDrop::Update(float dt) {
    spin += dt * 90.0f;
}

void LootDrop::Draw() const {
    const ItemDef* def = GetItemDef(itemId);
    Color c = GOLD;
    if (def) {
        if (def->category == "material") c = SKYBLUE;
        if (def->category == "relic") c = PURPLE;
        if (def->category == "weapon") c = ORANGE;
        if (def->category == "consumable") c = LIME;
        if (def->rare) c = VIOLET;
    }
    Vector3 pos = {position.x, 0.45f + std::sin(spin * DEG2RAD) * 0.08f, position.z};
    DrawCube(pos, 0.55f, 0.55f, 0.55f, c);
    DrawCubeWires(pos, 0.58f, 0.58f, 0.58f, WHITE);
}
