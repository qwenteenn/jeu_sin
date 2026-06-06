#include "Player.h"

void Player::ChooseClass(ClassType type, const UpgradeData& upgrades, const Inventory& inventory) {
    classType = type;
    ApplyUpgrades(upgrades, inventory);
    hp = maxHp;
    resource = maxResource;
}

void Player::ApplyUpgrades(const UpgradeData& upgrades, const Inventory& inventory) {
    maxHp = 100.0f + upgrades.Level("vitality") * 10.0f;
    defense = upgrades.Level("defense") * 0.03f;
    speed = 7.0f * (1.0f + upgrades.Level("speed") * 0.02f);

    switch (classType) {
        case ClassType::Mage:
            baseDamage = 17.0f * (1.0f + upgrades.Level("magic") * 0.05f);
            maxResource = 110.0f + upgrades.Level("mana") * 10.0f;
            break;
        case ClassType::Necromancer:
            baseDamage = 14.0f * (1.0f + upgrades.Level("magic") * 0.04f + upgrades.Level("necromancy") * 0.03f);
            maxResource = 70.0f + upgrades.Level("necromancy") * 8.0f;
            break;
        case ClassType::Fighter:
            maxHp += 45.0f;
            baseDamage = 20.0f * (1.0f + upgrades.Level("strength") * 0.05f);
            maxResource = 0.0f;
            defense += 0.08f;
            break;
        case ClassType::Monk:
            baseDamage = 13.0f * (1.0f + upgrades.Level("strength") * 0.03f);
            maxResource = 100.0f + upgrades.Level("ki") * 10.0f;
            speed *= 1.10f;
            break;
    }

    if (inventory.Count("armor_light") > 0) defense += 0.05f;
    hp = std::min(hp, maxHp);
    resource = std::min(resource, maxResource);
}

void Player::RespawnAt(Vector3 pos) {
    position = pos;
    hp = maxHp;
    resource = maxResource;
    primaryCooldown = 0.0f;
    secondaryCooldown = 0.0f;
    dashCooldown = 0.0f;
    invulnTimer = 1.0f;
    castingReturn = false;
    returnCastTimer = 0.0f;
}

void Player::Update(float dt) {
    primaryCooldown = std::max(0.0f, primaryCooldown - dt);
    secondaryCooldown = std::max(0.0f, secondaryCooldown - dt);
    dashCooldown = std::max(0.0f, dashCooldown - dt);
    invulnTimer = std::max(0.0f, invulnTimer - dt);
    if (classType == ClassType::Mage) GainResource(8.0f * dt);
    if (classType == ClassType::Monk) GainResource(4.0f * dt);
}

bool Player::IsDead() const {
    return hp <= 0.0f;
}

void Player::TakeDamage(float amount, DamageType type, const UpgradeData& upgrades) {
    if (invulnTimer > 0.0f) return;
    float reduction = defense;
    if (type == DamageType::Trap) reduction += upgrades.Level("trap_resist") * 0.05f;
    reduction = std::min(reduction, 0.75f);
    hp -= amount * (1.0f - reduction);
    hp = std::max(0.0f, hp);
    invulnTimer = 0.35f;
    if (castingReturn) {
        castingReturn = false;
        returnCastTimer = 0.0f;
    }
}

void Player::Heal(float amount) {
    hp = std::min(maxHp, hp + amount);
}

bool Player::SpendResource(float amount) {
    if (maxResource <= 0.0f) return true;
    if (resource < amount) return false;
    resource -= amount;
    return true;
}

void Player::GainResource(float amount) {
    if (maxResource <= 0.0f) return;
    resource = std::min(maxResource, resource + amount);
}

float Player::WeaponDamageBonus(const Inventory& inventory) const {
    const ItemDef* def = GetItemDef(inventory.equippedWeapon);
    return def ? def->damageBonus : 0.0f;
}

float Player::AttackRange(const Inventory& inventory) const {
    float range = 2.0f;
    if (classType == ClassType::Mage || classType == ClassType::Necromancer) range = 16.0f;
    const ItemDef* def = GetItemDef(inventory.equippedWeapon);
    if (def) range += def->rangeBonus;
    return range;
}

float Player::AttackCooldown(const Inventory& inventory) const {
    float cd = 0.55f;
    switch (classType) {
        case ClassType::Mage: cd = 0.55f; break;
        case ClassType::Necromancer: cd = 0.65f; break;
        case ClassType::Fighter: cd = 0.48f; break;
        case ClassType::Monk: cd = 0.32f; break;
    }
    const ItemDef* def = GetItemDef(inventory.equippedWeapon);
    if (def) cd *= (1.0f - def->attackSpeedBonus);
    return std::max(0.15f, cd);
}

Color Player::ClassColor() const {
    switch (classType) {
        case ClassType::Mage: return SKYBLUE;
        case ClassType::Necromancer: return PURPLE;
        case ClassType::Fighter: return ORANGE;
        case ClassType::Monk: return GOLD;
    }
    return WHITE;
}
