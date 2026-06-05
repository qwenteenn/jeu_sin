#pragma once
#include "Common.h"
#include "Inventory.h"

class Shop {
public:
    Shop() = default;
    explicit Shop(std::vector<std::string> ids);
    void SetItems(std::vector<std::string> ids);
    const std::vector<std::string>& Items() const;
    bool Buy(size_t index, Inventory& inventory) const;
private:
    std::vector<std::string> itemIds_;
};

class Vendor {
public:
    int Sell(size_t index, int amount, Inventory& inventory) const;
    int SellAllCommon(Inventory& inventory) const;
    std::vector<std::string> SellableIds(const Inventory& inventory) const;
};
