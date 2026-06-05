#include "CraftingStation.h"

CraftingStation::CraftingStation() {
    recipes_ = {
        {"bone_powder", "5 os anciens -> poudre d'os maudite", {{"ancient_bone", 5}}, "cursed_bone_powder", 1, 20},
        {"stone_core", "3 fragments de golem -> noyau de pierre", {{"golem_fragment", 3}}, "stone_core", 1, 40},
        {"soul_crystal", "4 essences spectrales -> cristal d'ame", {{"spectral_essence", 4}}, "soul_crystal", 1, 50},
        {"ancient_relic", "10 reliques cassees -> relique ancienne", {{"broken_relic", 10}}, "ancient_relic", 1, 100},
        {"return_crystal", "1 relique + 2 cristaux d'ame -> cristal retour stable", {{"ancient_relic", 1}, {"soul_crystal", 2}}, "stable_return_crystal", 1, 200},
        {"guardian_blade", "Coeur du Gardien + reliques -> lame rare", {{"boss_guardian_core", 1}, {"ancient_relic", 2}}, "rare_guardian_blade", 1, 250}
    };
}

const std::vector<Recipe>& CraftingStation::Recipes() const {
    return recipes_;
}

bool CraftingStation::CanCraft(const Recipe& recipe, const Inventory& inventory) const {
    if (inventory.gold < recipe.goldCost) return false;
    for (const auto& ing : recipe.input) {
        if (inventory.Count(ing.id) < ing.amount) return false;
    }
    return true;
}

bool CraftingStation::Craft(const Recipe& recipe, Inventory& inventory) const {
    if (!CanCraft(recipe, inventory)) return false;
    inventory.SpendGold(recipe.goldCost);
    for (const auto& ing : recipe.input) inventory.Remove(ing.id, ing.amount);
    inventory.Add(recipe.outputId, recipe.outputAmount);
    return true;
}
