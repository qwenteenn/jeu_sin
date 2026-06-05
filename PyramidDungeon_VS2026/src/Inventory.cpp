#include "Inventory.h"

void Inventory::AddGold(int amount) {
    gold = std::max(0, gold + amount);
}

bool Inventory::SpendGold(int amount) {
    if (gold < amount) return false;
    gold -= amount;
    return true;
}

void Inventory::Add(const std::string& id, int amount) {
    if (amount <= 0) return;
    if (id == "gold") { AddGold(amount); return; }
    if (id == "ancient_fragments") { ancientFragments += amount; return; }
    items[id] += amount;
}

bool Inventory::Remove(const std::string& id, int amount) {
    if (amount <= 0) return true;
    if (id == "gold") return SpendGold(amount);
    if (id == "ancient_fragments") {
        if (ancientFragments < amount) return false;
        ancientFragments -= amount;
        return true;
    }
    auto it = items.find(id);
    if (it == items.end() || it->second < amount) return false;
    it->second -= amount;
    if (it->second <= 0) items.erase(it);
    if (equippedWeapon == id && Count(id) <= 0) equippedWeapon.clear();
    return true;
}

int Inventory::Count(const std::string& id) const {
    if (id == "gold") return gold;
    if (id == "ancient_fragments") return ancientFragments;
    auto it = items.find(id);
    return it == items.end() ? 0 : it->second;
}

bool Inventory::Has(const std::string& id, int amount) const {
    return Count(id) >= amount;
}

bool Inventory::EquipWeapon(const std::string& id) {
    const ItemDef* def = GetItemDef(id);
    if (!def || def->category != "weapon" || Count(id) <= 0) return false;
    equippedWeapon = id;
    return true;
}

int Inventory::SellItem(const std::string& id, int amount) {
    const ItemDef* def = GetItemDef(id);
    if (!def || amount <= 0) return 0;
    const int owned = Count(id);
    const int sold = std::min(owned, amount);
    if (sold <= 0) return 0;
    if (!Remove(id, sold)) return 0;
    const int earned = def->sellPrice * sold;
    AddGold(earned);
    return earned;
}

int Inventory::SellAllCommonLoot() {
    int earned = 0;
    std::vector<std::pair<std::string, int>> snapshot = SortedItems();
    for (auto& [id, count] : snapshot) {
        const ItemDef* def = GetItemDef(id);
        if (!def) continue;
        if (def->autoSell && def->sellPrice > 0 && (def->category == "loot" || def->category == "material")) {
            earned += SellItem(id, count);
        }
    }
    return earned;
}

bool Inventory::DepositToChest(const std::string& id, int amount) {
    if (Remove(id, amount)) {
        chest[id] += amount;
        return true;
    }
    return false;
}

bool Inventory::WithdrawFromChest(const std::string& id, int amount) {
    auto it = chest.find(id);
    if (it == chest.end() || it->second < amount) return false;
    it->second -= amount;
    if (it->second <= 0) chest.erase(it);
    Add(id, amount);
    return true;
}

std::vector<std::pair<std::string, int>> Inventory::SortedItems() const {
    std::vector<std::pair<std::string, int>> out(items.begin(), items.end());
    std::sort(out.begin(), out.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
    return out;
}

std::vector<std::pair<std::string, int>> Inventory::SortedChest() const {
    std::vector<std::pair<std::string, int>> out(chest.begin(), chest.end());
    std::sort(out.begin(), out.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
    return out;
}
