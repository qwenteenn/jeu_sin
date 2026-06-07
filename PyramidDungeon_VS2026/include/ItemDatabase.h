#pragma once
#include "Common.h"

struct ItemDef {
    std::string id;
    std::string name;
    std::string category;
    int buyPrice = 0;
    int sellPrice = 0;
    bool autoSell = true;
    bool rare = false;
    float damageBonus = 0.0f;
    float attackSpeedBonus = 0.0f;
    float rangeBonus = 0.0f;
    std::string classHint;
};

const std::unordered_map<std::string, ItemDef>& ItemDB();
const ItemDef* GetItemDef(const std::string& id);
std::vector<std::string> WeaponShopItems();
std::vector<std::string> ConsumableShopItems();
std::vector<std::string> VendorLootOrder();
std::vector<std::string> AllItemIds();
