#pragma once
#include "Common.h"
#include "ItemDatabase.h"

class Inventory {
public:
    int gold = 250;
    int ancientFragments = 0;
    std::unordered_map<std::string, int> items;
    std::unordered_map<std::string, int> chest;
    std::string equippedWeapon;

    void AddGold(int amount);
    bool SpendGold(int amount);
    void Add(const std::string& id, int amount = 1);
    bool Remove(const std::string& id, int amount = 1);
    int Count(const std::string& id) const;
    bool Has(const std::string& id, int amount = 1) const;
    bool EquipWeapon(const std::string& id);
    int SellItem(const std::string& id, int amount = 1);
    int SellAllCommonLoot();
    bool DepositToChest(const std::string& id, int amount = 1);
    bool WithdrawFromChest(const std::string& id, int amount = 1);
    std::vector<std::pair<std::string, int>> SortedItems() const;
    std::vector<std::pair<std::string, int>> SortedChest() const;
};
