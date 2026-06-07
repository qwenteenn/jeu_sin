#include "ItemDatabase.h"

const std::unordered_map<std::string, ItemDef>& ItemDB() {
    static std::unordered_map<std::string, ItemDef> db = {
        {"potion_health", {"potion_health", "Potion de soin", "consumable", 25, 8, false, false}},
        {"potion_mana", {"potion_mana", "Potion de mana", "consumable", 25, 8, false, false}},
        {"return_stone", {"return_stone", "Pierre de Retour", "consumable", 100, 25, false, false}},
        {"stable_return_crystal", {"stable_return_crystal", "Cristal de Retour Stable", "consumable", 0, 250, false, true}},
        {"anti_mob_bomb", {"anti_mob_bomb", "Bombe anti-mob", "consumable", 80, 20, false, false}},
        {"defense_talisman", {"defense_talisman", "Talisman défensif", "relic", 250, 75, false, true}},
        {"ancient_key", {"ancient_key", "Clé ancienne", "key", 120, 40, false, false}},

        {"weapon_mage_staff", {"weapon_mage_staff", "Bâton élémentaire", "weapon", 150, 45, false, false, 8, 0.05f, 2.0f, "mage"}},
        {"weapon_necro_grimoire", {"weapon_necro_grimoire", "Grimoire sombre", "weapon", 180, 55, false, false, 7, 0.04f, 2.0f, "necromancer"}},
        {"weapon_short_sword", {"weapon_short_sword", "Épée courte", "weapon", 150, 45, false, false, 10, 0.10f, 0.3f, "fighter"}},
        {"weapon_heavy_axe", {"weapon_heavy_axe", "Hache lourde", "weapon", 220, 70, false, false, 18, -0.12f, 0.2f, "fighter"}},
        {"weapon_spear", {"weapon_spear", "Lance de garde", "weapon", 190, 60, false, false, 12, 0.0f, 1.0f, "fighter"}},
        {"weapon_monk_gloves", {"weapon_monk_gloves", "Gants de moine", "weapon", 150, 45, false, false, 7, 0.18f, 0.2f, "monk"}},
        {"weapon_monk_staff", {"weapon_monk_staff", "Bâton de moine", "weapon", 190, 60, false, false, 10, 0.08f, 0.9f, "monk"}},
        {"armor_light", {"armor_light", "Armure légère", "armor", 200, 55, false, false}},

        {"scarab_shell", {"scarab_shell", "Carapace de scarabée", "loot", 0, 7, true, false}},
        {"weak_venom", {"weak_venom", "Venin faible", "loot", 0, 9, true, false}},
        {"ancient_bone", {"ancient_bone", "Os ancien", "loot", 0, 10, true, false}},
        {"rusted_weapon_fragment", {"rusted_weapon_fragment", "Fragment d'arme rouillée", "loot", 0, 12, true, false}},
        {"cursed_bandage", {"cursed_bandage", "Bandelette maudite", "loot", 0, 16, true, false}},
        {"ancient_dust", {"ancient_dust", "Poussière ancienne", "loot", 0, 14, true, false}},
        {"spectral_essence", {"spectral_essence", "Essence spectrale", "material", 0, 22, false, true}},
        {"soul_fragment", {"soul_fragment", "Fragment d'âme", "material", 0, 24, false, true}},
        {"weak_crystal", {"weak_crystal", "Cristal faible", "material", 0, 18, false, false}},
        {"golem_fragment", {"golem_fragment", "Fragment de golem", "material", 0, 28, false, true}},
        {"stone_core", {"stone_core", "Noyau de pierre", "material", 0, 90, false, true}},
        {"broken_relic", {"broken_relic", "Relique cassée", "loot", 0, 35, false, true}},
        {"ancient_relic", {"ancient_relic", "Relique ancienne", "relic", 0, 300, false, true}},
        {"cursed_bone_powder", {"cursed_bone_powder", "Poudre d'os maudite", "material", 0, 55, false, true}},
        {"soul_crystal", {"soul_crystal", "Cristal d'âme", "material", 0, 130, false, true}},
        {"boss_guardian_core", {"boss_guardian_core", "Coeur du Gardien", "material", 0, 550, false, true}},
        {"rare_guardian_blade", {"rare_guardian_blade", "Lame du Gardien", "weapon", 0, 420, false, true, 30, 0.05f, 0.6f, "fighter"}}
    };
    return db;
}

const ItemDef* GetItemDef(const std::string& id) {
    auto& db = ItemDB();
    auto it = db.find(id);
    if (it == db.end()) return nullptr;
    return &it->second;
}

std::vector<std::string> WeaponShopItems() {
    return {
        "weapon_mage_staff", "weapon_necro_grimoire", "weapon_short_sword", "weapon_heavy_axe",
        "weapon_spear", "weapon_monk_gloves", "weapon_monk_staff", "armor_light"
    };
}

std::vector<std::string> ConsumableShopItems() {
    return {"potion_health", "potion_mana", "return_stone", "anti_mob_bomb", "defense_talisman", "ancient_key"};
}

std::vector<std::string> VendorLootOrder() {
    return {
        "scarab_shell", "weak_venom", "ancient_bone", "rusted_weapon_fragment", "cursed_bandage",
        "ancient_dust", "weak_crystal", "broken_relic", "spectral_essence", "soul_fragment",
        "golem_fragment", "stone_core", "cursed_bone_powder", "soul_crystal", "ancient_relic", "boss_guardian_core"
    };
}

std::vector<std::string> AllItemIds() {
    std::vector<std::string> ids;
    for (const auto& [id, def] : ItemDB()) ids.push_back(id);
    std::sort(ids.begin(), ids.end());
    return ids;
}
