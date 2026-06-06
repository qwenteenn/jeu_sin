#pragma once
#include "Common.h"
#include "Inventory.h"

struct Ingredient {
    std::string id;
    int amount = 1;
};

struct Recipe {
    std::string id;
    std::string name;
    std::vector<Ingredient> input;
    std::string outputId;
    int outputAmount = 1;
    int goldCost = 0;
};

class CraftingStation {
public:
    CraftingStation();
    const std::vector<Recipe>& Recipes() const;
    bool CanCraft(const Recipe& recipe, const Inventory& inventory) const;
    bool Craft(const Recipe& recipe, Inventory& inventory) const;
private:
    std::vector<Recipe> recipes_;
};
