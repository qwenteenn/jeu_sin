#pragma once
#include "Common.h"
#include "Inventory.h"
#include "Types.h"
#include "UpgradeStation.h"

class Player {
public:
    Vector3 position {0, 0, 0};
    Vector3 facing {0, 0, 1};
    ClassType classType = ClassType::Mage;

    float hp = 100.0f;
    float maxHp = 100.0f;
    float resource = 100.0f;
    float maxResource = 100.0f;
    float baseDamage = 16.0f;
    float defense = 0.0f;
    float speed = 7.0f;
    float radius = 0.65f;
    float primaryCooldown = 0.0f;
    float secondaryCooldown = 0.0f;
    float dashCooldown = 0.0f;
    float invulnTimer = 0.0f;
    float returnCastTimer = 0.0f;
    bool castingReturn = false;

    void ChooseClass(ClassType type, const UpgradeData& upgrades, const Inventory& inventory);
    void ApplyUpgrades(const UpgradeData& upgrades, const Inventory& inventory);
    void RespawnAt(Vector3 pos);
    void Update(float dt);
    bool IsDead() const;
    void TakeDamage(float amount, DamageType type, const UpgradeData& upgrades);
    void Heal(float amount);
    bool SpendResource(float amount);
    void GainResource(float amount);
    float WeaponDamageBonus(const Inventory& inventory) const;
    float AttackRange(const Inventory& inventory) const;
    float AttackCooldown(const Inventory& inventory) const;
    Color ClassColor() const;
};
