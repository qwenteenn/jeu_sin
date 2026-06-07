#pragma once
#include "Common.h"

enum class GameMode {
    MainMenu,
    Hub,
    Dungeon,
    ClassSelection,
    WeaponShop,
    ItemShop,
    Vendor,
    Crafting,
    Upgrades,
    Chest,
    Inventory,
    Pause,
    Options,
    GameOver,
    Victory
};

enum class ClassType {
    Mage,
    Necromancer,
    Fighter,
    Monk
};

enum class DamageType {
    Physical,
    Magical,
    Dark,
    Spiritual,
    Trap
};

enum class EnemyType {
    Scarab,
    Skeleton,
    Mummy,
    Specter,
    Golem,
    SummonedSkeleton
};

enum class RoomType {
    Entrance,
    Combat,
    Trap,
    Chest,
    Rest,
    Cursed,
    MiniBoss,
    Transition,
    Boss
};

enum class TrapType {
    Spikes,
    FireJet,
    ArrowWall,
    Quicksand,
    CrumblingFloor,
    ChestBomb
};

enum class ProjectileOwner {
    Player,
    Enemy
};

inline std::string ClassName(ClassType type) {
    switch (type) {
        case ClassType::Mage: return "Mage élémentaire";
        case ClassType::Necromancer: return "Nécromancien";
        case ClassType::Fighter: return "Combattant";
        case ClassType::Monk: return "Moine";
    }
    return "Inconnu";
}

inline std::string EnemyName(EnemyType type) {
    switch (type) {
        case EnemyType::Scarab: return "Scarabee geant";
        case EnemyType::Skeleton: return "Squelette";
        case EnemyType::Mummy: return "Momie";
        case EnemyType::Specter: return "Spectre";
        case EnemyType::Golem: return "Golem de pierre";
        case EnemyType::SummonedSkeleton: return "Squelette invoqué";
    }
    return "Ennemi";
}
