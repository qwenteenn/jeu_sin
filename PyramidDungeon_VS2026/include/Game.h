#pragma once
#include "Common.h"
#include "Boss.h"
#include "CraftingStation.h"
#include "Dungeon.h"
#include "Inventory.h"
#include "Player.h"
#include "Projectile.h"
#include "SaveSystem.h"
#include "Shop.h"
#include "UI.h"
#include "UpgradeStation.h"

struct HubZone {
    std::string id;
    std::string name;
    Vector3 position {0, 0, 0};
    Color color = WHITE;
};

class Game {
public:
    Game();
    void Run();

private:
    GameMode mode_ = GameMode::Hub;
    GameMode previousMode_ = GameMode::Hub;
    Camera3D camera_ {};
    float cameraYaw_ = 0.0f;
    float cameraPitch_ = 0.0f;
    bool mouseCaptured_ = true;
    bool shouldQuit_ = false;

    SaveData save_;
    SaveSystem saveSystem_;
    Player player_;
    Dungeon dungeon_;
    Boss boss_;
    std::vector<Projectile> projectiles_;

    Shop weaponShop_;
    Shop itemShop_;
    Vendor vendor_;
    CraftingStation crafting_;
    UpgradeStation upgradeStation_;
    std::vector<HubZone> hubZones_;

    int selectedIndex_ = 0;
    bool chestSide_ = false;
    std::string message_;
    float messageTimer_ = 0.0f;

    void Init();
    void Update(float dt);
    void Draw();
    void DrawHub();
    void DrawPlayer() const;
    void DrawHubZones() const;
    void UpdateCamera(float dt);
    void UpdateMovement(float dt);
    void UpdateHub(float dt);
    void UpdateDungeon(float dt);
    void UpdateMenu(float dt);
    void UpdateProjectiles(float dt);
    void UpdateCombatInput();
    void UpdateConsumableInput();
    void TryInteractHub();
    void TryInteractDungeon();
    void EnterDungeon(int floor);
    void ReturnToHub(const std::string& reason, bool deathPenalty);
    void SetMessage(const std::string& msg, float duration = 3.0f);
    void SaveNow();
    bool IsInGameplay() const;
    bool IsWalkable(Vector3 pos) const;
    bool IsWalkableHub(Vector3 pos) const;
    bool IsWalkableDungeon(Vector3 pos) const;
    Vector3 ForwardFlat() const;
    Vector3 ForwardView() const;
    void DealMeleeDamage(float damage, float range, DamageType type);
    void SpawnPlayerProjectile(float damage, DamageType type, Color color, float speed, float radius);
    void UseSecondarySkill();
    void UseReturnItem();
    void ApplyLootPickup();
    void OnEnemyKilled(const Enemy& enemy);
    void CheckBossSpawn();
    void AdvanceFloor();
    void HandleMenuMouseInput();
    void ActivateMenuSelection(int action = UI::ACTION_PRIMARY);
    int MaxMenuIndex() const;
    void ChangeMode(GameMode mode);
};
