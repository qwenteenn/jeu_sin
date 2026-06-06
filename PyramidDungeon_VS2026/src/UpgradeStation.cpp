#include "UpgradeStation.h"

int UpgradeData::Level(const std::string& id) const {
    auto it = levels.find(id);
    return it == levels.end() ? 0 : it->second;
}

void UpgradeData::SetLevel(const std::string& id, int level) {
    levels[id] = std::max(0, level);
}

UpgradeStation::UpgradeStation() {
    upgrades_ = {
        {"vitality", "Vitalité", "+10 vie max par niveau", 90, "ancient_bone", 1, 20},
        {"strength", "Force", "+5% dégâts physiques", 120, "golem_fragment", 1, 15},
        {"magic", "Puissance magique", "+5% dégâts magiques", 120, "weak_crystal", 2, 15},
        {"defense", "Défense", "-3% dégâts reçus", 130, "stone_core", 1, 12},
        {"speed", "Vitesse", "+2% vitesse", 100, "scarab_shell", 4, 10},
        {"mana", "Maîtrise du mana", "+10 mana max", 100, "spectral_essence", 1, 15},
        {"ki", "Maîtrise du ki", "+10 ki max", 100, "cursed_bone_powder", 1, 15},
        {"necromancy", "Nécromancie", "Invocations plus résistantes", 130, "soul_fragment", 2, 12},
        {"loot", "Instinct du pilleur", "+2% chance de drop rare", 180, "ancient_relic", 1, 10},
        {"trap_resist", "Peau de pierre", "+5% résistance aux pièges", 150, "golem_fragment", 2, 10}
    };
}

const std::vector<UpgradeDef>& UpgradeStation::Upgrades() const {
    return upgrades_;
}

int UpgradeStation::GoldCost(const UpgradeDef& def, const UpgradeData& data) const {
    const int lvl = data.Level(def.id);
    return def.baseGold + lvl * def.baseGold / 2;
}

int UpgradeStation::MaterialCost(const UpgradeDef& def, const UpgradeData& data) const {
    const int lvl = data.Level(def.id);
    return def.baseMaterial + lvl;
}

bool UpgradeStation::CanBuy(const UpgradeDef& def, const UpgradeData& data, const Inventory& inventory) const {
    if (data.Level(def.id) >= def.maxLevel) return false;
    if (inventory.gold < GoldCost(def, data)) return false;
    if (!def.materialId.empty() && inventory.Count(def.materialId) < MaterialCost(def, data)) return false;
    return true;
}

bool UpgradeStation::Buy(const UpgradeDef& def, UpgradeData& data, Inventory& inventory) const {
    if (!CanBuy(def, data, inventory)) return false;
    inventory.SpendGold(GoldCost(def, data));
    if (!def.materialId.empty()) inventory.Remove(def.materialId, MaterialCost(def, data));
    data.SetLevel(def.id, data.Level(def.id) + 1);
    return true;
}
