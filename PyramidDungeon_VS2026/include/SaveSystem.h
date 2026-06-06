#pragma once
#include "Common.h"
#include "Inventory.h"
#include "Types.h"
#include "UpgradeStation.h"

struct SaveData {
    Inventory inventory;
    UpgradeData upgrades;
    ClassType classType = ClassType::Mage;
    int maxUnlockedFloor = 1;
};

class SaveSystem {
public:
    explicit SaveSystem(std::string path = "save.txt");
    bool Load(SaveData& data) const;
    bool Save(const SaveData& data) const;
    const std::string& Path() const;
private:
    std::string path_;
};
