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

enum class InputAction {
    MoveForward,
    MoveBack,
    MoveLeft,
    MoveRight,
    Dash,
    Interact,
    Potion,
    Return,
    Inventory,
    Pause,
    QuickFloor1,
    QuickFloor2,
    QuickFloor3,
    Jump,
    Sneak,
    Count
};

struct KeyBinding {
    InputAction action = InputAction::MoveForward;
    std::string name;
    std::string description;
    KeyboardKey key = KEY_NULL;
    KeyboardKey altKey = KEY_NULL;
};

struct GameSettings {
    std::vector<KeyBinding> bindings;
    int masterVolume = 80;
    int musicVolume = 70;
    int sfxVolume = 80;
    int graphicsQuality = 1;
    int targetFps = 144;
    int fov = 75;
    bool showFps = true;
};

class Game {
public:
    Game();
    void Run();

private:
    GameMode mode_ = GameMode::MainMenu;
    GameMode previousMode_ = GameMode::MainMenu;
    Camera3D camera_ {};
    float cameraYaw_ = 0.0f;
    float cameraPitch_ = 0.0f;
    float verticalVelocity_ = 0.0f;
    bool mouseCaptured_ = true;
    bool shouldQuit_ = false;
    bool borderlessWindowed_ = false;
    Vector2 windowedPosition_ {};
    int windowedWidth_ = SCREEN_WIDTH;
    int windowedHeight_ = SCREEN_HEIGHT;
    RenderTexture2D sceneTarget_ {};
    int sceneRenderWidth_ = 0;
    int sceneRenderHeight_ = 0;

    SaveData save_;
    SaveSystem saveSystem_;
    GameSettings settings_;
    GameSettings pendingSettings_;
    std::string settingsPath_ = "settings.txt";
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
    int optionsSection_ = 0;
    int optionsScroll_ = 0;
    int optionsSliderDrag_ = -1;
    bool editingFpsValue_ = false;
    std::string fpsValueBuffer_;
    bool pendingBorderlessWindowed_ = false;
    int waitingForBindIndex_ = -1;
    bool optionsFromPause_ = false;
    bool chestSide_ = false;
    std::string message_;
    float messageTimer_ = 0.0f;

    void Init();
    void Update(float dt);
    void Draw();
    void RenderWorldToTarget();
    void EnsureSceneRenderTarget();
    void DrawMainMenu();
    void DrawOptionsMenu();
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
    void LoadSettings();
    void SaveSettings();
    void ApplySettings();
    void ApplyPendingSettings();
    bool HasPendingSettingsChanges() const;
    void ResetDefaultSettings();
    void ToggleBorderlessWindowedMode();
    bool IsActionPressed(InputAction action) const;
    bool IsActionDown(InputAction action) const;
    KeyBinding* Binding(InputAction action);
    const KeyBinding* Binding(InputAction action) const;
    std::string KeyLabel(KeyboardKey key) const;
    bool IsInGameplay() const;
    bool IsWalkable(Vector3 pos) const;
    bool IsWalkableHub(Vector3 pos) const;
    bool IsWalkableDungeon(Vector3 pos) const;
    Vector3 ForwardFlat() const;
    Vector3 ForwardView() const;
    float EyeHeight() const;
    void DealMeleeDamage(float damage, float range, DamageType type);
    void SpawnPlayerProjectile(float damage, DamageType type, Color color, float speed, float radius);
    void UseSecondarySkill();
    void UseReturnItem();
    void ApplyLootPickup();
    void OnEnemyKilled(const Enemy& enemy);
    void CheckBossSpawn();
    void AdvanceFloor();
    void HandleMenuMouseInput();
    bool HandleOptionsSliderMouseInput();
    bool HandleFpsValueEdit();
    void BeginFpsValueEdit();
    bool GetOptionsSliderValue(int localIndex, int& value, int& minValue, int& maxValue,
                               bool& stepped, int& stepCount) const;
    bool SetOptionsSliderValue(int localIndex, int value);
    void ActivateMenuSelection(int action = UI::ACTION_PRIMARY);
    void ActivateOptionsSelection();
    void AdjustOptionsSelection(int delta);
    int MaxMenuIndex() const;
    void ChangeMode(GameMode mode);
};
