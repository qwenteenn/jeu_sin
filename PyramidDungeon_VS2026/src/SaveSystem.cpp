#include "SaveSystem.h"

SaveSystem::SaveSystem(std::string path) : path_(std::move(path)) {}

const std::string& SaveSystem::Path() const { return path_; }

bool SaveSystem::Load(SaveData& data) const {
    std::ifstream file(path_);
    if (!file.is_open()) return false;
    data.inventory.items.clear();
    data.inventory.chest.clear();
    data.upgrades.levels.clear();

    std::string key;
    while (file >> key) {
        if (key == "gold") file >> data.inventory.gold;
        else if (key == "ancientFragments") file >> data.inventory.ancientFragments;
        else if (key == "class") {
            int c = 0; file >> c; data.classType = static_cast<ClassType>(std::max(0, std::min(3, c)));
        }
        else if (key == "maxFloor") file >> data.maxUnlockedFloor;
        else if (key == "equipped") file >> data.inventory.equippedWeapon;
        else if (key == "item") {
            std::string id; int amount = 0; file >> id >> amount; if (amount > 0) data.inventory.items[id] = amount;
        }
        else if (key == "chest") {
            std::string id; int amount = 0; file >> id >> amount; if (amount > 0) data.inventory.chest[id] = amount;
        }
        else if (key == "upgrade") {
            std::string id; int lvl = 0; file >> id >> lvl; data.upgrades.SetLevel(id, lvl);
        }
    }
    data.maxUnlockedFloor = std::max(1, std::min(3, data.maxUnlockedFloor));
    return true;
}

bool SaveSystem::Save(const SaveData& data) const {
    std::ofstream file(path_, std::ios::trunc);
    if (!file.is_open()) return false;
    file << "gold " << data.inventory.gold << "\n";
    file << "ancientFragments " << data.inventory.ancientFragments << "\n";
    file << "class " << static_cast<int>(data.classType) << "\n";
    file << "maxFloor " << data.maxUnlockedFloor << "\n";
    file << "equipped " << (data.inventory.equippedWeapon.empty() ? "none" : data.inventory.equippedWeapon) << "\n";
    for (const auto& [id, amount] : data.inventory.items) file << "item " << id << " " << amount << "\n";
    for (const auto& [id, amount] : data.inventory.chest) file << "chest " << id << " " << amount << "\n";
    for (const auto& [id, lvl] : data.upgrades.levels) file << "upgrade " << id << " " << lvl << "\n";
    return true;
}
