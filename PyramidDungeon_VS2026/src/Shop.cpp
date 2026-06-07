#include "Shop.h"

Shop::Shop(std::vector<std::string> ids) : itemIds_(std::move(ids)) {}

void Shop::SetItems(std::vector<std::string> ids) {
    itemIds_ = std::move(ids);
}

const std::vector<std::string>& Shop::Items() const {
    return itemIds_;
}

bool Shop::Buy(size_t index, Inventory& inventory) const {
    if (index >= itemIds_.size()) return false;
    const ItemDef* def = GetItemDef(itemIds_[index]);
    if (!def || def->buyPrice <= 0) return false;
    if (!inventory.SpendGold(def->buyPrice)) return false;
    inventory.Add(def->id, 1);
    if (def->category == "weapon" && inventory.equippedWeapon.empty()) inventory.EquipWeapon(def->id);
    return true;
}

int Vendor::Sell(size_t index, int amount, Inventory& inventory) const {
    auto ids = SellableIds(inventory);
    if (index >= ids.size()) return 0;
    return inventory.SellItem(ids[index], amount);
}

int Vendor::SellAllCommon(Inventory& inventory) const {
    return inventory.SellAllCommonLoot();
}

std::vector<std::string> Vendor::SellableIds(const Inventory& inventory) const {
    std::vector<std::string> out;
    for (const auto& id : VendorLootOrder()) {
        if (inventory.Count(id) > 0) out.push_back(id);
    }
    return out;
}
