#pragma once
#include "Common.h"
#include "Inventory.h"

struct UpgradeData {
    std::unordered_map<std::string, int> levels;

    int Level(const std::string& id) const;
    void SetLevel(const std::string& id, int level);
};

struct UpgradeDef {
    std::string id;
    std::string name;
    std::string description;
    int baseGold = 0;
    std::string materialId;
    int baseMaterial = 0;
    int maxLevel = 10;
};

class UpgradeStation {
public:
    UpgradeStation();
    const std::vector<UpgradeDef>& Upgrades() const;
    int GoldCost(const UpgradeDef& def, const UpgradeData& data) const;
    int MaterialCost(const UpgradeDef& def, const UpgradeData& data) const;
    bool CanBuy(const UpgradeDef& def, const UpgradeData& data, const Inventory& inventory) const;
    bool Buy(const UpgradeDef& def, UpgradeData& data, Inventory& inventory) const;

private:
    std::vector<UpgradeDef> upgrades_;
};
