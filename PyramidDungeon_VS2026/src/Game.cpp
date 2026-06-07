#include "Game.h"

#include <limits>

namespace {
std::string FitLabel(const std::string& text, int fontSize, int maxWidth) {
    if (MeasureText(text.c_str(), fontSize) <= maxWidth) return text;
    std::string fitted = text;
    while (!fitted.empty() && MeasureText((fitted + "...").c_str(), fontSize) > maxWidth) fitted.pop_back();
    return fitted.empty() ? std::string("...") : fitted + "...";
}

void DrawWrappedText(const std::string& text, Rectangle bounds, int fontSize, int lineHeight, Color color) {
    std::istringstream words(text);
    std::string word;
    std::string line;
    int y = static_cast<int>(bounds.y);
    const int bottom = static_cast<int>(bounds.y + bounds.height);

    while (words >> word && y + fontSize <= bottom) {
        const std::string candidate = line.empty() ? word : line + " " + word;
        if (!line.empty() && MeasureText(candidate.c_str(), fontSize) > bounds.width) {
            DrawText(line.c_str(), static_cast<int>(bounds.x), y, fontSize, color);
            line = word;
            y += lineHeight;
        } else {
            line = candidate;
        }
    }
    if (!line.empty() && y + fontSize <= bottom) {
        DrawText(FitLabel(line, fontSize, static_cast<int>(bounds.width)).c_str(),
                 static_cast<int>(bounds.x), y, fontSize, color);
    }
}

Rectangle OptionsRowRectangle(int localIndex, int scrollOffset, int rowCount) {
    const int screenW = GetScreenWidth();
    const int screenH = GetScreenHeight();
    const int margin = std::max(16, std::min(32, screenW / 60));
    const int titleH = screenH < 700 ? 58 : 68;
    const int tabH = screenH < 700 ? 44 : 52;
    const int rowH = screenH < 700 ? 42 : 50;
    const int innerW = screenW - margin * 2;
    const int leftW = std::max(520, static_cast<int>(innerW * 0.66f));
    const int contentY = margin + titleH + 12 + tabH + 10;
    const int contentH = std::max(250, screenH - contentY - margin - 30);
    const int sectionHeaderH = screenH < 700 ? 42 : 48;
    const int listTop = contentY + sectionHeaderH + 7;
    const int visibleRows = std::max(1, (contentY + contentH - 8 - listTop) / rowH);
    const int maxScroll = std::max(0, rowCount - visibleRows);
    const int safeScroll = std::max(0, std::min(maxScroll, scrollOffset));
    const int visibleIndex = localIndex - safeScroll;
    if (visibleIndex < 0 || visibleIndex >= visibleRows) return Rectangle{};

    const int rowWidth = leftW - (maxScroll > 0 ? 20 : 10);
    return Rectangle {
        static_cast<float>(margin + 5),
        static_cast<float>(listTop + visibleIndex * rowH),
        static_cast<float>(rowWidth),
        static_cast<float>(rowH - 4)
    };
}

Rectangle OptionsSliderRectangle(Rectangle row) {
    const float x = row.x + row.width * 0.47f;
    const float right = row.x + row.width - 104.0f;
    return Rectangle{x, row.y + row.height * 0.60f, std::max(90.0f, right - x), 6.0f};
}

Rectangle OptionsSliderValueRectangle(Rectangle row) {
    return Rectangle {
        row.x + row.width - 96.0f,
        row.y + 7.0f,
        86.0f,
        row.height - 14.0f
    };
}
}

Game::Game()
    : saveSystem_("save.txt"),
      weaponShop_(WeaponShopItems()),
      itemShop_(ConsumableShopItems()) {}

void Game::Run() {
    Init();
    while (!shouldQuit_ && !WindowShouldClose()) {
        const float dt = GetFrameTime();
        Update(dt);
        EnsureSceneRenderTarget();
        RenderWorldToTarget();
        BeginDrawing();
        ClearBackground(Color{9, 9, 13, 255});
        Draw();
        EndDrawing();
    }
    SaveNow();
    if (sceneTarget_.id != 0) UnloadRenderTexture(sceneTarget_);
    if (IsAudioDeviceReady()) CloseAudioDevice();
    CloseWindow();
}

void Game::Init() {
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pyramid Dungeon - C++ raylib");
    InitAudioDevice();
    SetWindowMinSize(960, 540);
    windowedPosition_ = GetWindowPosition();
    windowedWidth_ = GetScreenWidth();
    windowedHeight_ = GetScreenHeight();
    SetExitKey(KEY_NULL);
    ResetDefaultSettings();
    LoadSettings();
    ApplySettings();
    pendingSettings_ = settings_;
    pendingBorderlessWindowed_ = borderlessWindowed_;
    EnableCursor();
    mouseCaptured_ = false;

    if (!saveSystem_.Load(save_)) {
        save_.inventory.gold = 250;
        save_.inventory.Add("potion_health", 3);
        save_.inventory.Add("return_stone", 1);
        save_.inventory.Add("weapon_short_sword", 1);
        save_.inventory.equippedWeapon = "weapon_short_sword";
        save_.classType = ClassType::Fighter;
        save_.maxUnlockedFloor = 1;
        SaveNow();
    }
    if (save_.inventory.equippedWeapon == "none") save_.inventory.equippedWeapon.clear();

    player_.position = {0, 0, 0};
    player_.ChooseClass(save_.classType, save_.upgrades, save_.inventory);

    camera_.position = {0, 1.55f, 0};
    camera_.target = {0, 1.55f, 1};
    camera_.up = {0, 1, 0};
    camera_.fovy = static_cast<float>(settings_.fov);
    camera_.projection = CAMERA_PERSPECTIVE;

    hubZones_ = {
        {"class", "Choix de classe", {0, 0, -11}, SKYBLUE},
        {"weapons", "Marchand d'armes", {10, 0, -5}, ORANGE},
        {"items", "Marchand d'objets", {-10, 0, -5}, LIME},
        {"vendor", "Vente de loots", {10, 0, 6}, GOLD},
        {"craft", "Transformation", {-10, 0, 6}, PURPLE},
        {"upgrade", "Améliorations", {0, 0, 8}, VIOLET},
        {"chest", "Coffre personnel", {-10, 0, -13}, BROWN},
        {"portal", "Portail du donjon", {0, 0, 16}, BLUE}
    };

    SetMessage("Bienvenue au Sanctuaire de l'Entrée. E pour interagir avec les zones.", 4.0f);
}

bool Game::IsInGameplay() const {
    return mode_ == GameMode::Hub || mode_ == GameMode::Dungeon;
}

void Game::Update(float dt) {
    if (messageTimer_ > 0.0f) {
        messageTimer_ -= dt;
        if (messageTimer_ <= 0.0f) message_.clear();
    }

    if (IsKeyPressed(KEY_F1)) {
        mouseCaptured_ = !mouseCaptured_;
        if (mouseCaptured_) DisableCursor(); else EnableCursor();
    }
    if (IsKeyPressed(KEY_F11)) {
        ToggleBorderlessWindowedMode();
    }
    if (IsInGameplay() && IsActionPressed(InputAction::Pause)) {
        previousMode_ = mode_;
        ChangeMode(GameMode::Pause);
        return;
    }

    if (IsInGameplay()) {
        player_.Update(dt);
        UpdateCamera(dt);
        UpdateMovement(dt);
        UpdateConsumableInput();
        if (mode_ == GameMode::Hub) UpdateHub(dt);
        if (mode_ == GameMode::Dungeon) UpdateDungeon(dt);
    } else {
        UpdateMenu(dt);
    }
}

void Game::UpdateCamera(float dt) {
    (void)dt;
    if (mouseCaptured_) {
        Vector2 delta = GetMouseDelta();
        cameraYaw_ -= delta.x * 0.0035f;
        cameraPitch_ -= delta.y * 0.0025f;
        cameraPitch_ = std::max(-1.25f, std::min(1.25f, cameraPitch_));
    }

    const Vector3 eye {player_.position.x, player_.position.y + EyeHeight(), player_.position.z};
    const Vector3 view = ForwardView();
    camera_.position = eye;
    camera_.target = Vector3Add(eye, view);
    player_.facing = ForwardFlat();
}

void Game::UpdateMovement(float dt) {
    const bool grounded = player_.position.y <= WORLD_Y + 0.001f;
    if (grounded) {
        player_.position.y = WORLD_Y;
        verticalVelocity_ = std::max(0.0f, verticalVelocity_);
    }
    if (grounded && IsActionPressed(InputAction::Jump) && !IsActionDown(InputAction::Sneak)) {
        verticalVelocity_ = 7.3f;
    }

    Vector3 forward = ForwardFlat();
    Vector3 right {std::cos(cameraYaw_), 0, -std::sin(cameraYaw_)};
    Vector3 move {0, 0, 0};

    if (IsActionDown(InputAction::MoveForward)) move = Vector3Add(move, forward);
    if (IsActionDown(InputAction::MoveBack)) move = Vector3Subtract(move, forward);
    if (IsActionDown(InputAction::MoveRight)) move = Vector3Subtract(move, right);
    if (IsActionDown(InputAction::MoveLeft)) move = Vector3Add(move, right);

    if (Vector3Length(move) > 0.01f) {
        move = Vector3Normalize(move);
        player_.facing = move;
        const float speed = player_.speed * (IsActionDown(InputAction::Sneak) ? 0.45f : 1.0f);
        Vector3 next = AddScaled(player_.position, move, speed * dt);
        if (IsWalkable(next)) player_.position = next;
    }

    if (IsActionPressed(InputAction::Dash) && player_.dashCooldown <= 0.0f) {
        Vector3 dir = Vector3Length(move) > 0.01f ? move : ForwardFlat();
        float dashDistance = player_.classType == ClassType::Monk ? 5.2f : 3.1f;
        if (player_.classType == ClassType::Monk && player_.SpendResource(12.0f)) dashDistance = 6.2f;
        Vector3 next = AddScaled(player_.position, dir, dashDistance);
        if (IsWalkable(next)) player_.position = next;
        player_.dashCooldown = player_.classType == ClassType::Monk ? 0.55f : 1.0f;
        player_.invulnTimer = 0.22f;
        if (player_.classType == ClassType::Monk) player_.GainResource(8.0f);
    }

    verticalVelocity_ -= 19.5f * dt;
    player_.position.y += verticalVelocity_ * dt;
    if (player_.position.y < WORLD_Y) {
        player_.position.y = WORLD_Y;
        verticalVelocity_ = 0.0f;
    }
}

Vector3 Game::ForwardFlat() const {
    return {std::sin(cameraYaw_), 0.0f, std::cos(cameraYaw_)};
}

Vector3 Game::ForwardView() const {
    return {
        std::sin(cameraYaw_) * std::cos(cameraPitch_),
        std::sin(cameraPitch_),
        std::cos(cameraYaw_) * std::cos(cameraPitch_)
    };
}

float Game::EyeHeight() const {
    return IsActionDown(InputAction::Sneak) ? 1.12f : 1.55f;
}

bool Game::IsWalkable(Vector3 pos) const {
    if (mode_ == GameMode::Hub) return IsWalkableHub(pos);
    if (mode_ == GameMode::Dungeon) return IsWalkableDungeon(pos);
    return true;
}

bool Game::IsWalkableHub(Vector3 pos) const {
    return pos.x > -24 && pos.x < 24 && pos.z > -20 && pos.z < 22;
}

bool Game::IsWalkableDungeon(Vector3 pos) const {
    for (const auto& r : dungeon_.rooms) if (r.Contains(pos)) return true;
    for (size_t i = 0; i + 1 < dungeon_.rooms.size(); ++i) {
        Vector3 a = dungeon_.rooms[i].center;
        Vector3 b = dungeon_.rooms[i + 1].center;
        Vector3 center {(a.x + b.x) * 0.5f, 0, (a.z + b.z) * 0.5f};
        Vector3 half {std::abs(b.x - a.x) * 0.5f + 0.4f, 1, 2.0f};
        if (PointInsideXZ(pos, center, half)) return true;
    }
    return false;
}

void Game::UpdateHub(float dt) {
    (void)dt;
    if (IsActionPressed(InputAction::Interact)) TryInteractHub();
    if (IsActionPressed(InputAction::Inventory)) { previousMode_ = mode_; ChangeMode(GameMode::Inventory); }
    if (IsActionPressed(InputAction::QuickFloor1)) EnterDungeon(1);
    if (IsActionPressed(InputAction::QuickFloor2) && save_.maxUnlockedFloor >= 2) EnterDungeon(2);
    if (IsActionPressed(InputAction::QuickFloor3) && save_.maxUnlockedFloor >= 3) EnterDungeon(3);
}

void Game::TryInteractHub() {
    for (const auto& z : hubZones_) {
        if (DistanceXZ(player_.position, z.position) < 2.6f) {
            if (z.id == "class") ChangeMode(GameMode::ClassSelection);
            else if (z.id == "weapons") ChangeMode(GameMode::WeaponShop);
            else if (z.id == "items") ChangeMode(GameMode::ItemShop);
            else if (z.id == "vendor") ChangeMode(GameMode::Vendor);
            else if (z.id == "craft") ChangeMode(GameMode::Crafting);
            else if (z.id == "upgrade") ChangeMode(GameMode::Upgrades);
            else if (z.id == "chest") { chestSide_ = false; ChangeMode(GameMode::Chest); }
            else if (z.id == "portal") EnterDungeon(save_.maxUnlockedFloor);
            return;
        }
    }
    SetMessage("Aucune zone interactive proche. Marche sur un marqueur, cette technologie avance vite.", 2.5f);
}

void Game::EnterDungeon(int floor) {
    floor = std::max(1, std::min(save_.maxUnlockedFloor, floor));
    dungeon_.GenerateFloor(floor);
    projectiles_.clear();
    boss_ = Boss{};
    player_.ApplyUpgrades(save_.upgrades, save_.inventory);
    player_.position = dungeon_.StartPositionForFloor(floor);
    player_.position.y = WORLD_Y;
    verticalVelocity_ = 0.0f;
    player_.hp = player_.maxHp;
    player_.resource = player_.maxResource;
    mode_ = GameMode::Dungeon;
    DisableCursor();
    mouseCaptured_ = true;
    SetMessage("Étage " + std::to_string(floor) + " : descente dans la pyramide.", 3.0f);
}

void Game::UpdateDungeon(float dt) {
    dungeon_.UpdateRoomStates(player_.position);
    Room* room = dungeon_.CurrentRoom(player_.position);
    if (room) {
        for (auto& trap : room->traps) trap.Update(dt, player_, save_.upgrades);
    }

    CheckBossSpawn();
    UpdateCombatInput();

    for (auto& enemy : dungeon_.enemies) {
        enemy.Update(dt, player_, projectiles_, dungeon_.enemies);
    }
    if (boss_.alive) boss_.Update(dt, player_, dungeon_.enemies, projectiles_);

    UpdateProjectiles(dt);

    for (auto& enemy : dungeon_.enemies) {
        if (!enemy.alive && enemy.hp <= 0.0f) {
            OnEnemyKilled(enemy);
            enemy.hp = 999999.0f;
        }
    }
    if (!boss_.alive && boss_.maxHp > 0.0f && boss_.hp <= 0.0f && !dungeon_.bossDefeated && dungeon_.bossSpawned) {
        dungeon_.bossDefeated = true;
        auto drops = boss_.GenerateLoot();
        dungeon_.loot.insert(dungeon_.loot.end(), drops.begin(), drops.end());
        SetMessage("Gardien vaincu. La pyramide a pris une claque monumentale.", 5.0f);
        save_.maxUnlockedFloor = std::max(save_.maxUnlockedFloor, 3);
        SaveNow();
    }

    ApplyLootPickup();
    TryInteractDungeon();

    if (player_.IsDead()) {
        ReturnToHub("Mort dans le donjon. Retour au sanctuaire avec pénalité.", true);
    }
    if (IsActionPressed(InputAction::Inventory)) { previousMode_ = mode_; ChangeMode(GameMode::Inventory); }
}

void Game::UpdateCombatInput() {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && player_.primaryCooldown <= 0.0f) {
        const float damage = player_.baseDamage + player_.WeaponDamageBonus(save_.inventory);
        const float range = player_.AttackRange(save_.inventory);
        if (player_.classType == ClassType::Mage) {
            SpawnPlayerProjectile(damage, DamageType::Magical, SKYBLUE, 18.0f, 0.28f);
        } else if (player_.classType == ClassType::Necromancer) {
            SpawnPlayerProjectile(damage, DamageType::Dark, PURPLE, 15.0f, 0.30f);
        } else if (player_.classType == ClassType::Fighter) {
            DealMeleeDamage(damage * 1.15f, range, DamageType::Physical);
        } else if (player_.classType == ClassType::Monk) {
            DealMeleeDamage(damage, range, DamageType::Spiritual);
            player_.GainResource(5.0f);
        }
        player_.primaryCooldown = player_.AttackCooldown(save_.inventory);
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && player_.secondaryCooldown <= 0.0f) {
        UseSecondarySkill();
    }
}

void Game::DealMeleeDamage(float damage, float range, DamageType type) {
    (void)type;
    bool hit = false;
    const Vector3 attackDir = ForwardFlat();
    for (auto& enemy : dungeon_.enemies) {
        if (!enemy.alive || enemy.allied) continue;
        if (DistanceXZ(player_.position, enemy.position) <= range + enemy.radius) {
            Vector3 toEnemy = DirectionXZ(player_.position, enemy.position);
            if (Vector3DotProduct(attackDir, toEnemy) > -0.25f) {
                enemy.TakeDamage(damage);
                hit = true;
            }
        }
    }
    if (boss_.alive && DistanceXZ(player_.position, boss_.position) <= range + boss_.radius) {
        boss_.TakeDamage(damage);
        hit = true;
    }
    if (hit && player_.classType == ClassType::Monk) player_.GainResource(8.0f);
}

void Game::SpawnPlayerProjectile(float damage, DamageType type, Color color, float speed, float radius) {
    Projectile p;
    p.owner = ProjectileOwner::Player;
    p.damage = damage;
    p.damageType = type;
    p.radius = radius;
    const Vector3 dir = ForwardView();
    p.position = Vector3Add(camera_.position, Vector3Scale(dir, 0.8f));
    p.velocity = Vector3Scale(dir, speed);
    p.lifetime = 2.2f;
    p.color = color;
    projectiles_.push_back(p);
}

void Game::UseSecondarySkill() {
    const float damage = player_.baseDamage + player_.WeaponDamageBonus(save_.inventory);
    if (player_.classType == ClassType::Mage) {
        if (!player_.SpendResource(35.0f)) { SetMessage("Pas assez de mana.", 1.5f); return; }
        for (auto& enemy : dungeon_.enemies) {
            if (enemy.alive && !enemy.allied && DistanceXZ(player_.position, enemy.position) < 5.0f) enemy.TakeDamage(damage * 1.8f);
        }
        if (boss_.alive && DistanceXZ(player_.position, boss_.position) < 6.0f) boss_.TakeDamage(damage * 1.5f);
        player_.secondaryCooldown = 4.0f;
        SetMessage("Explosion élémentaire.", 1.2f);
    } else if (player_.classType == ClassType::Necromancer) {
        if (!player_.SpendResource(35.0f)) { SetMessage("Pas assez d'énergie d'âme.", 1.5f); return; }
        dungeon_.enemies.emplace_back(EnemyType::SummonedSkeleton, AddScaled(player_.position, player_.facing, 1.7f), dungeon_.currentFloor, true);
        float bonus = save_.upgrades.Level("necromancy") * 6.0f;
        dungeon_.enemies.back().hp += bonus;
        dungeon_.enemies.back().maxHp += bonus;
        player_.secondaryCooldown = 8.0f;
        SetMessage("Squelette invoqué.", 1.5f);
    } else if (player_.classType == ClassType::Fighter) {
        Vector3 next = AddScaled(player_.position, player_.facing, 5.5f);
        if (IsWalkable(next)) player_.position = next;
        DealMeleeDamage(damage * 2.1f, 2.2f, DamageType::Physical);
        player_.secondaryCooldown = 4.0f;
        SetMessage("Charge du combattant.", 1.2f);
    } else if (player_.classType == ClassType::Monk) {
        if (!player_.SpendResource(25.0f)) { SetMessage("Pas assez de ki.", 1.5f); return; }
        SpawnPlayerProjectile(damage * 1.4f, DamageType::Spiritual, GOLD, 17.0f, 0.35f);
        player_.secondaryCooldown = 2.2f;
        SetMessage("Onde de ki.", 1.2f);
    }
}

void Game::UpdateConsumableInput() {
    if (IsActionPressed(InputAction::Potion)) {
        if (save_.inventory.Remove("potion_health", 1)) {
            player_.Heal(45.0f);
            SetMessage("Potion de soin utilisée.", 1.5f);
            SaveNow();
        } else SetMessage("Aucune potion de soin.", 1.5f);
    }
    if (mode_ == GameMode::Dungeon && IsActionPressed(InputAction::Return)) UseReturnItem();
}

void Game::UseReturnItem() {
    if (boss_.alive) {
        SetMessage("Impossible d'utiliser un retour pendant le boss. Évidemment, ce serait trop simple.", 2.5f);
        return;
    }
    if (save_.inventory.Remove("stable_return_crystal", 1)) {
        ReturnToHub("Cristal stable utilisé : retour instantané.", false);
        return;
    }
    if (save_.inventory.Count("return_stone") <= 0) {
        SetMessage("Aucune Pierre de Retour.", 1.8f);
        return;
    }
    if (!player_.castingReturn) {
        player_.castingReturn = true;
        player_.returnCastTimer = 3.0f;
        SetMessage("Incantation de retour : 3 secondes. Ne te fais pas frapper, concept révolutionnaire.", 3.0f);
    }
}

void Game::TryInteractDungeon() {
    Room* room = dungeon_.CurrentRoom(player_.position);
    if (!room) return;

    if (player_.castingReturn) {
        player_.returnCastTimer -= GetFrameTime();
        if (player_.returnCastTimer <= 0.0f) {
            if (save_.inventory.Remove("return_stone", 1)) ReturnToHub("Pierre de Retour utilisée.", false);
            player_.castingReturn = false;
        }
    }

    if (!IsActionPressed(InputAction::Interact)) return;

    if (room->type == RoomType::Chest && !room->chestOpened && DistanceXZ(player_.position, room->center) < 2.2f) {
        room->chestOpened = true;
        const int gold = RandomInt(60, 140) * dungeon_.currentFloor;
        save_.inventory.AddGold(gold);
        save_.inventory.Add("broken_relic", 1);
        if (RandomFloat(0, 1) < 0.45f) save_.inventory.Add("return_stone", 1);
        if (RandomFloat(0, 1) < 0.25f) save_.inventory.Add("ancient_key", 1);
        SetMessage("Coffre ouvert : " + std::to_string(gold) + " or + loot.", 2.5f);
        SaveNow();
    }
    if (room->type == RoomType::Rest && !room->rewardTaken && DistanceXZ(player_.position, room->center) < 2.0f) {
        room->rewardTaken = true;
        player_.Heal(player_.maxHp * 0.35f);
        player_.GainResource(player_.maxResource * 0.35f);
        SetMessage("Autel de repos active.", 2.0f);
    }
    if (room->type == RoomType::Transition && DistanceXZ(player_.position, room->center) < 2.8f) {
        if (!room->transitionUnlocked) {
            SetMessage("Le Cercle de Descente est verrouillé. Nettoie l'étage avant de supplier le sol.", 2.5f);
            return;
        }
        if (dungeon_.currentFloor == 2 && save_.inventory.Count("ancient_key") <= 0) {
            SetMessage("Une Clé ancienne est nécessaire pour ouvrir la descente.", 2.5f);
            return;
        }
        if (dungeon_.currentFloor == 2) save_.inventory.Remove("ancient_key", 1);
        AdvanceFloor();
    }
}

void Game::AdvanceFloor() {
    if (dungeon_.currentFloor >= 3) return;
    const int next = dungeon_.currentFloor + 1;
    save_.maxUnlockedFloor = std::max(save_.maxUnlockedFloor, next);
    SaveNow();
    EnterDungeon(next);
}

void Game::CheckBossSpawn() {
    Room* room = dungeon_.CurrentRoom(player_.position);
    if (!room || room->type != RoomType::Boss || dungeon_.bossSpawned) return;
    boss_.Spawn(room->center, dungeon_.currentFloor);
    dungeon_.bossSpawned = true;
    SetMessage("Le Gardien de la Pyramide se réveille.", 3.0f);
}

void Game::UpdateProjectiles(float dt) {
    for (auto& p : projectiles_) {
        if (!p.alive) continue;
        p.Update(dt);
        if (p.owner == ProjectileOwner::Player) {
            for (auto& enemy : dungeon_.enemies) {
                if (!enemy.alive || enemy.allied) continue;
                if (DistanceXZ(p.position, enemy.position) < p.radius + enemy.radius) {
                    enemy.TakeDamage(p.damage);
                    p.alive = false;
                    break;
                }
            }
            if (p.alive && boss_.alive && DistanceXZ(p.position, boss_.position) < p.radius + boss_.radius) {
                boss_.TakeDamage(p.damage);
                p.alive = false;
            }
        } else {
            if (DistanceXZ(p.position, player_.position) < p.radius + player_.radius) {
                player_.TakeDamage(p.damage, p.damageType, save_.upgrades);
                p.alive = false;
            }
        }
    }
    projectiles_.erase(std::remove_if(projectiles_.begin(), projectiles_.end(), [](const Projectile& p) { return !p.alive; }), projectiles_.end());
}

void Game::ApplyLootPickup() {
    for (auto& l : dungeon_.loot) {
        if (!l.alive) continue;
        l.Update(GetFrameTime());
        if (DistanceXZ(player_.position, l.position) < 1.4f) {
            save_.inventory.Add(l.itemId, l.amount);
            const ItemDef* def = GetItemDef(l.itemId);
            if (l.itemId == "gold") SetMessage("+" + std::to_string(l.amount) + " or", 0.8f);
            else SetMessage("Ramasse : " + std::string(def ? def->name : l.itemId) + " x" + std::to_string(l.amount), 1.4f);
            l.alive = false;
        }
    }
    dungeon_.loot.erase(std::remove_if(dungeon_.loot.begin(), dungeon_.loot.end(), [](const LootDrop& l) { return !l.alive; }), dungeon_.loot.end());
}

void Game::OnEnemyKilled(const Enemy& enemy) {
    float lootBonus = save_.upgrades.Level("loot") * 0.02f;
    auto drops = enemy.GenerateLoot(lootBonus);
    dungeon_.loot.insert(dungeon_.loot.end(), drops.begin(), drops.end());
    if (player_.classType == ClassType::Necromancer) player_.GainResource(18.0f);
}

void Game::ReturnToHub(const std::string& reason, bool deathPenalty) {
    if (deathPenalty) save_.inventory.gold = static_cast<int>(save_.inventory.gold * 0.75f);
    mode_ = GameMode::Hub;
    DisableCursor();
    mouseCaptured_ = true;
    player_.ApplyUpgrades(save_.upgrades, save_.inventory);
    player_.RespawnAt({0, 0, 0});
    verticalVelocity_ = 0.0f;
    dungeon_.enemies.clear();
    dungeon_.loot.clear();
    projectiles_.clear();
    boss_ = Boss{};
    SaveNow();
    SetMessage(reason, 4.0f);
}

void Game::UpdateMenu(float dt) {
    (void)dt;
    if (mode_ == GameMode::Options && editingFpsValue_) {
        HandleFpsValueEdit();
        return;
    }
    if (mode_ == GameMode::Options && waitingForBindIndex_ >= 0) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            waitingForBindIndex_ = -1;
            SetMessage("Reconfiguration annulée.", 1.0f);
            return;
        }
        int key = GetKeyPressed();
        if (key != KEY_NULL && waitingForBindIndex_ < static_cast<int>(settings_.bindings.size())) {
            settings_.bindings[waitingForBindIndex_].key = static_cast<KeyboardKey>(key);
            waitingForBindIndex_ = -1;
            SaveSettings();
            SetMessage("Touche modifiée.", 1.2f);
        }
        return;
    }

    if (IsKeyPressed(KEY_ESCAPE) || (mode_ == GameMode::Inventory && IsKeyPressed(KEY_TAB))) {
        if (mode_ == GameMode::MainMenu) shouldQuit_ = true;
        else if (mode_ == GameMode::Options && optionsFromPause_) ChangeMode(GameMode::Pause);
        else ChangeMode(previousMode_);
        return;
    }
    if (mode_ == GameMode::Inventory) {
        const int columns = UI::InventoryGridColumns();
        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) selectedIndex_ = std::min(MaxMenuIndex(), selectedIndex_ + 1);
        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_A)) selectedIndex_ = std::max(0, selectedIndex_ - 1);
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) selectedIndex_ = std::min(MaxMenuIndex(), selectedIndex_ + columns);
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_Z)) selectedIndex_ = std::max(0, selectedIndex_ - columns);
    } else {
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) selectedIndex_ = std::min(MaxMenuIndex(), selectedIndex_ + 1);
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_Z)) selectedIndex_ = std::max(0, selectedIndex_ - 1);
    }

    if (mode_ == GameMode::Options) {
        const float wheel = GetMouseWheelMove();
        if (wheel != 0.0f) {
            if (selectedIndex_ < 3) selectedIndex_ = 3;
            selectedIndex_ = std::max(3, std::min(MaxMenuIndex(), selectedIndex_ + (wheel > 0.0f ? -1 : 1)));
        }
    }

    const bool sliderHandled = mode_ == GameMode::Options && HandleOptionsSliderMouseInput();
    if (!sliderHandled) HandleMenuMouseInput();
    if (mode_ == GameMode::Options && (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_A))) AdjustOptionsSelection(-1);
    if (mode_ == GameMode::Options && (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))) AdjustOptionsSelection(1);
    if (mode_ == GameMode::Vendor && IsKeyPressed(KEY_V)) ActivateMenuSelection(UI::ACTION_VENDOR_SELL_ALL);
    if (mode_ == GameMode::Chest && IsKeyPressed(KEY_C)) ActivateMenuSelection(UI::ACTION_CHEST_TOGGLE_SIDE);
    if (IsKeyPressed(KEY_ENTER)) ActivateMenuSelection();

    selectedIndex_ = std::min(selectedIndex_, MaxMenuIndex());
}

void Game::HandleMenuMouseInput() {
    const int hovered = UI::HoveredMenuIndex();
    if (hovered >= 0) selectedIndex_ = std::min(MaxMenuIndex(), hovered);
    if (UI::WasMenuItemClicked()) ActivateMenuSelection(UI::HoveredMenuAction());
}

bool Game::GetOptionsSliderValue(int localIndex, int& value, int& minValue, int& maxValue,
                                 bool& stepped, int& stepCount) const {
    stepped = false;
    stepCount = 0;
    if (optionsSection_ == 1 && localIndex >= 0 && localIndex <= 2) {
        value = localIndex == 0 ? pendingSettings_.masterVolume :
                (localIndex == 1 ? pendingSettings_.musicVolume : pendingSettings_.sfxVolume);
        minValue = 0;
        maxValue = 100;
        return true;
    }
    if (optionsSection_ != 2) return false;

    if (localIndex == 0) {
        value = pendingSettings_.graphicsQuality;
        minValue = 0;
        maxValue = 3;
        stepped = true;
        stepCount = 4;
        return true;
    }
    if (localIndex == 1) {
        value = pendingSettings_.targetFps;
        minValue = 0;
        maxValue = 360;
        return true;
    }
    if (localIndex == 2) {
        value = pendingSettings_.fov;
        minValue = 30;
        maxValue = 120;
        return true;
    }
    return false;
}

bool Game::SetOptionsSliderValue(int localIndex, int value) {
    int currentValue = 0;
    int minValue = 0;
    int maxValue = 0;
    bool stepped = false;
    int stepCount = 0;
    if (!GetOptionsSliderValue(localIndex, currentValue, minValue, maxValue, stepped, stepCount)) return false;
    value = std::max(minValue, std::min(maxValue, value));
    if (value == currentValue) return false;

    if (optionsSection_ == 1) {
        if (localIndex == 0) pendingSettings_.masterVolume = value;
        else if (localIndex == 1) pendingSettings_.musicVolume = value;
        else pendingSettings_.sfxVolume = value;
    } else if (localIndex == 0) {
        pendingSettings_.graphicsQuality = value;
    } else if (localIndex == 1) {
        pendingSettings_.targetFps = value;
    } else if (localIndex == 2) {
        pendingSettings_.fov = value;
    }

    return true;
}

bool Game::HandleOptionsSliderMouseInput() {
    if (optionsSection_ == 0) {
        optionsSliderDrag_ = -1;
        return false;
    }

    const int rowCount = optionsSection_ == 1 ? 5 : 7;
    const int sliderCount = optionsSection_ == 1 ? 3 : 3;
    const Vector2 mouse = GetMousePosition();
    bool consumed = false;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        optionsSliderDrag_ = -1;
        for (int local = 0; local < sliderCount; ++local) {
            const Rectangle row = OptionsRowRectangle(local, optionsScroll_, rowCount);
            if (row.width <= 0.0f || !CheckCollisionPointRec(mouse, row)) continue;
            selectedIndex_ = 3 + local;
            consumed = true;

            if (optionsSection_ == 2 && local == 1 &&
                CheckCollisionPointRec(mouse, OptionsSliderValueRectangle(row))) {
                BeginFpsValueEdit();
                break;
            }

            const Rectangle track = OptionsSliderRectangle(row);
            const Rectangle dragArea {track.x - 12.0f, row.y, track.width + 24.0f, row.height};
            if (CheckCollisionPointRec(mouse, dragArea)) optionsSliderDrag_ = local;
            break;
        }
    }

    if (optionsSliderDrag_ >= 0 && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        const Rectangle row = OptionsRowRectangle(optionsSliderDrag_, optionsScroll_, rowCount);
        const Rectangle track = OptionsSliderRectangle(row);
        int value = 0;
        int minValue = 0;
        int maxValue = 0;
        bool stepped = false;
        int stepCount = 0;
        if (track.width > 0.0f &&
            GetOptionsSliderValue(optionsSliderDrag_, value, minValue, maxValue, stepped, stepCount)) {
            const float normalized = Clamp01((mouse.x - track.x) / track.width);
            int newValue = minValue + static_cast<int>(std::round(normalized * (maxValue - minValue)));
            if (stepped && stepCount > 1) {
                const int nearestStep = static_cast<int>(std::round(normalized * (stepCount - 1)));
                newValue = minValue + static_cast<int>(std::round(
                    nearestStep * (maxValue - minValue) / static_cast<float>(stepCount - 1)));
            }
            SetOptionsSliderValue(optionsSliderDrag_, newValue);
        }
        selectedIndex_ = 3 + optionsSliderDrag_;
        consumed = true;
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) optionsSliderDrag_ = -1;
    return consumed;
}

void Game::BeginFpsValueEdit() {
    editingFpsValue_ = true;
    fpsValueBuffer_.clear();
    optionsSliderDrag_ = -1;
    selectedIndex_ = 4;
}

bool Game::HandleFpsValueEdit() {
    if (!editingFpsValue_) return false;

    if (IsKeyPressed(KEY_ESCAPE)) {
        editingFpsValue_ = false;
        fpsValueBuffer_.clear();
        SetMessage("Saisie FPS annulée.", 1.0f);
        return true;
    }

    int character = GetCharPressed();
    while (character > 0) {
        if (character >= '0' && character <= '9' && fpsValueBuffer_.size() < 10) {
            fpsValueBuffer_.push_back(static_cast<char>(character));
        }
        character = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE) && !fpsValueBuffer_.empty()) fpsValueBuffer_.pop_back();

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
        if (!fpsValueBuffer_.empty()) {
            const long long parsed = std::stoll(fpsValueBuffer_);
            pendingSettings_.targetFps = static_cast<int>(std::min(
                parsed, static_cast<long long>(std::numeric_limits<int>::max())));
        }
        editingFpsValue_ = false;
        fpsValueBuffer_.clear();
    }
    return true;
}

void Game::ActivateMenuSelection(int action) {
    if (mode_ == GameMode::MainMenu) {
        if (selectedIndex_ == 0) ChangeMode(GameMode::Hub);
        else if (selectedIndex_ == 1) { previousMode_ = GameMode::MainMenu; optionsFromPause_ = false; ChangeMode(GameMode::Options); }
        else if (selectedIndex_ == 2) shouldQuit_ = true;
    } else if (mode_ == GameMode::Options) {
        ActivateOptionsSelection();
    } else if (mode_ == GameMode::ClassSelection) {
        ClassType t = static_cast<ClassType>(selectedIndex_);
        save_.classType = t;
        player_.ChooseClass(t, save_.upgrades, save_.inventory);
        SaveNow();
        SetMessage("Classe choisie : " + ClassName(t), 2.0f);
        ChangeMode(GameMode::Hub);
    } else if (mode_ == GameMode::WeaponShop) {
        if (weaponShop_.Buy(selectedIndex_, save_.inventory)) { SaveNow(); SetMessage("Achat effectué.", 1.5f); }
        else SetMessage("Pas assez d'or ou achat impossible.", 1.8f);
    } else if (mode_ == GameMode::ItemShop) {
        if (itemShop_.Buy(selectedIndex_, save_.inventory)) { SaveNow(); SetMessage("Achat effectué.", 1.5f); }
        else SetMessage("Pas assez d'or ou achat impossible.", 1.8f);
    } else if (mode_ == GameMode::Vendor) {
        if (action == UI::ACTION_VENDOR_SELL_ALL) {
            int earned = vendor_.SellAllCommon(save_.inventory);
            SaveNow();
            SetMessage("Vente automatique : +" + std::to_string(earned) + " or", 2.0f);
        } else {
            int earned = vendor_.Sell(selectedIndex_, 1, save_.inventory);
            if (earned > 0) { SaveNow(); SetMessage("Vente : +" + std::to_string(earned) + " or", 1.5f); }
        }
    } else if (mode_ == GameMode::Crafting) {
        const auto& recipes = crafting_.Recipes();
        if (selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(recipes.size()) && crafting_.Craft(recipes[selectedIndex_], save_.inventory)) {
            SaveNow(); SetMessage("Transformation effectuée.", 1.5f);
        } else SetMessage("Ressources insuffisantes.", 1.6f);
    } else if (mode_ == GameMode::Upgrades) {
        const auto& ups = upgradeStation_.Upgrades();
        if (selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(ups.size()) && upgradeStation_.Buy(ups[selectedIndex_], save_.upgrades, save_.inventory)) {
            player_.ApplyUpgrades(save_.upgrades, save_.inventory);
            SaveNow(); SetMessage("Amélioration achetée.", 1.5f);
        } else SetMessage("Pas assez de ressources pour améliorer.", 1.8f);
    } else if (mode_ == GameMode::Inventory) {
        auto items = UI::InventoryDisplayItems(save_.inventory);
        if (selectedIndex_ < static_cast<int>(items.size())) {
            if (save_.inventory.EquipWeapon(items[selectedIndex_].first)) {
                player_.ApplyUpgrades(save_.upgrades, save_.inventory);
                SaveNow(); SetMessage("Arme équipée.", 1.2f);
            }
        }
    } else if (mode_ == GameMode::Chest) {
        if (action == UI::ACTION_CHEST_TOGGLE_SIDE) {
            chestSide_ = !chestSide_;
            selectedIndex_ = 0;
            return;
        }
        {
            auto list = chestSide_ ? save_.inventory.SortedChest() : save_.inventory.SortedItems();
            if (selectedIndex_ < static_cast<int>(list.size())) {
                bool ok = chestSide_ ? save_.inventory.WithdrawFromChest(list[selectedIndex_].first, 1) : save_.inventory.DepositToChest(list[selectedIndex_].first, 1);
                if (ok) { SaveNow(); SetMessage(chestSide_ ? "Objet retiré du coffre." : "Objet déposé au coffre.", 1.2f); }
            }
        }
    } else if (mode_ == GameMode::Pause) {
        if (selectedIndex_ == 0) {
            ChangeMode(previousMode_);
        } else if (selectedIndex_ == 1) {
            optionsFromPause_ = true;
            ChangeMode(GameMode::Options);
        } else if (selectedIndex_ == 2) {
            SaveNow();
            SetMessage("Partie sauvegardee.", 1.5f);
        } else if (selectedIndex_ == 3) {
            if (previousMode_ == GameMode::Dungeon) ReturnToHub("Retour au sanctuaire depuis le menu pause.", false);
            else ChangeMode(previousMode_);
        } else if (selectedIndex_ == 4) {
            ReturnToHub("Retour au menu principal.", false);
            previousMode_ = GameMode::MainMenu;
            optionsFromPause_ = false;
            ChangeMode(GameMode::MainMenu);
        } else if (selectedIndex_ == 5) {
            SaveNow();
            shouldQuit_ = true;
        }
    }
}

void Game::ActivateOptionsSelection() {
    if (selectedIndex_ < 3) {
        optionsSection_ = selectedIndex_;
        selectedIndex_ = 3;
        optionsScroll_ = 0;
        optionsSliderDrag_ = -1;
        editingFpsValue_ = false;
        fpsValueBuffer_.clear();
        return;
    }

    const int local = selectedIndex_ - 3;
    if (optionsSection_ == 0) {
        if (local < static_cast<int>(settings_.bindings.size())) {
            waitingForBindIndex_ = local;
            SetMessage("Appuie sur une touche pour : " + settings_.bindings[local].name + ". Échap annule.", 4.0f);
        } else if (local == static_cast<int>(settings_.bindings.size())) {
            ResetDefaultSettings();
            ApplySettings();
            SaveSettings();
            pendingSettings_ = settings_;
            SetMessage("Commandes par défaut restaurées.", 1.5f);
        } else {
            if (optionsFromPause_) ChangeMode(GameMode::Pause);
            else ChangeMode(previousMode_);
        }
    } else {
        const int applyIndex = optionsSection_ == 1 ? 3 : 5;
        const int backIndex = optionsSection_ == 1 ? 4 : 6;
        if (local == backIndex) {
            if (optionsFromPause_) ChangeMode(GameMode::Pause);
            else ChangeMode(previousMode_);
        }
        else if (local == applyIndex) ApplyPendingSettings();
        else if (optionsSection_ == 2 && local == 1) BeginFpsValueEdit();
        else AdjustOptionsSelection(1);
    }
}

void Game::AdjustOptionsSelection(int delta) {
    if (mode_ != GameMode::Options || selectedIndex_ < 3) return;
    const int local = selectedIndex_ - 3;
    if (optionsSection_ == 1) {
        if (local == 0) pendingSettings_.masterVolume = std::max(0, std::min(100, pendingSettings_.masterVolume + delta));
        else if (local == 1) pendingSettings_.musicVolume = std::max(0, std::min(100, pendingSettings_.musicVolume + delta));
        else if (local == 2) pendingSettings_.sfxVolume = std::max(0, std::min(100, pendingSettings_.sfxVolume + delta));
        else return;
    } else if (optionsSection_ == 2) {
        if (local == 0) pendingSettings_.graphicsQuality = std::max(0, std::min(3, pendingSettings_.graphicsQuality + delta));
        else if (local == 1) pendingSettings_.targetFps = std::max(0, pendingSettings_.targetFps + delta);
        else if (local == 2) pendingSettings_.fov = std::max(30, std::min(120, pendingSettings_.fov + delta));
        else if (local == 3) pendingSettings_.showFps = !pendingSettings_.showFps;
        else if (local == 4) pendingBorderlessWindowed_ = !pendingBorderlessWindowed_;
        else return;
    } else {
        return;
    }
}

int Game::MaxMenuIndex() const {
    switch (mode_) {
        case GameMode::MainMenu: return 2;
        case GameMode::Options:
            if (optionsSection_ == 0) return 3 + static_cast<int>(settings_.bindings.size()) + 1;
            if (optionsSection_ == 1) return 7;
            return 9;
        case GameMode::ClassSelection: return 3;
        case GameMode::WeaponShop: return std::max(0, static_cast<int>(weaponShop_.Items().size()) - 1);
        case GameMode::ItemShop: return std::max(0, static_cast<int>(itemShop_.Items().size()) - 1);
        case GameMode::Vendor: return std::max(0, static_cast<int>(vendor_.SellableIds(save_.inventory).size()) - 1);
        case GameMode::Crafting: return std::max(0, static_cast<int>(crafting_.Recipes().size()) - 1);
        case GameMode::Upgrades: return std::max(0, static_cast<int>(upgradeStation_.Upgrades().size()) - 1);
        case GameMode::Inventory: return std::max(0, static_cast<int>(UI::InventoryDisplayItems(save_.inventory).size()) - 1);
        case GameMode::Chest: return std::max(0, static_cast<int>((chestSide_ ? save_.inventory.SortedChest() : save_.inventory.SortedItems()).size()) - 1);
        case GameMode::Pause: return 5;
        default: return 0;
    }
}

void Game::ChangeMode(GameMode mode) {
    const GameMode oldMode = mode_;
    selectedIndex_ = 0;
    mode_ = mode;
    optionsSliderDrag_ = -1;
    editingFpsValue_ = false;
    fpsValueBuffer_.clear();
    if (mode_ == GameMode::Options) {
        optionsScroll_ = 0;
        if (oldMode != GameMode::Options) {
            pendingSettings_ = settings_;
            pendingBorderlessWindowed_ = borderlessWindowed_;
        }
    }
    if (IsInGameplay()) {
        DisableCursor();
        mouseCaptured_ = true;
    } else {
        EnableCursor();
        mouseCaptured_ = false;
    }
}

void Game::ToggleBorderlessWindowedMode() {
    if (!borderlessWindowed_) {
        windowedPosition_ = GetWindowPosition();
        windowedWidth_ = GetScreenWidth();
        windowedHeight_ = GetScreenHeight();

        const int monitor = GetCurrentMonitor();
        const Vector2 monitorPosition = GetMonitorPosition(monitor);
        SetWindowState(FLAG_WINDOW_UNDECORATED);
        SetWindowPosition(static_cast<int>(monitorPosition.x), static_cast<int>(monitorPosition.y));
        SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
        borderlessWindowed_ = true;
        SetMessage("Plein écran fenêtré sans bordure activé.", 2.0f);
    } else {
        ClearWindowState(FLAG_WINDOW_UNDECORATED);
        SetWindowSize(windowedWidth_, windowedHeight_);
        SetWindowPosition(static_cast<int>(windowedPosition_.x), static_cast<int>(windowedPosition_.y));
        borderlessWindowed_ = false;
        SetMessage("Mode fenêtré restauré.", 2.0f);
    }

    SetTargetFPS(settings_.targetFps);
    if (mode_ == GameMode::Options) pendingBorderlessWindowed_ = borderlessWindowed_;
}

void Game::SaveNow() {
    save_.classType = player_.classType;
    saveSystem_.Save(save_);
}

void Game::SetMessage(const std::string& msg, float duration) {
    message_ = msg;
    messageTimer_ = duration;
}

void Game::ResetDefaultSettings() {
    settings_.bindings = {
        {InputAction::MoveForward, "Avancer", "Se déplacer vers l'avant dans la direction du regard.", KEY_Z, KEY_W},
        {InputAction::MoveBack, "Reculer", "Se déplacer en arrière.", KEY_S, KEY_NULL},
        {InputAction::MoveLeft, "Gauche", "Pas latéral gauche.", KEY_Q, KEY_NULL},
        {InputAction::MoveRight, "Droite", "Pas latéral droit.", KEY_D, KEY_NULL},
        {InputAction::Jump, "Saut", "Sauter en première personne.", KEY_SPACE, KEY_NULL},
        {InputAction::Sneak, "Sneak", "Se déplacer lentement et baisser la caméra.", KEY_LEFT_SHIFT, KEY_RIGHT_SHIFT},
        {InputAction::Dash, "Dash", "Esquive rapide dans la direction de mouvement.", KEY_LEFT_CONTROL, KEY_RIGHT_CONTROL},
        {InputAction::Interact, "Interagir", "Utiliser une zone, coffre, autel ou cercle.", KEY_E, KEY_NULL},
        {InputAction::Potion, "Potion", "Utiliser une potion de soin.", KEY_A, KEY_NULL},
        {InputAction::Return, "Retour", "Utiliser une Pierre de Retour dans le donjon.", KEY_R, KEY_NULL},
        {InputAction::Inventory, "Inventaire", "Ouvrir ou fermer l'inventaire.", KEY_TAB, KEY_NULL},
        {InputAction::Pause, "Pause", "Ouvrir le menu pause.", KEY_ESCAPE, KEY_NULL},
        {InputAction::QuickFloor1, "Accès étage 1", "Entrer rapidement dans l'étage 1 depuis le sanctuaire.", KEY_ONE, KEY_NULL},
        {InputAction::QuickFloor2, "Accès étage 2", "Entrer rapidement dans l'étage 2 si débloqué.", KEY_TWO, KEY_NULL},
        {InputAction::QuickFloor3, "Accès étage 3", "Entrer rapidement dans l'étage 3 si débloqué.", KEY_THREE, KEY_NULL}
    };
    settings_.masterVolume = 80;
    settings_.musicVolume = 70;
    settings_.sfxVolume = 80;
    settings_.graphicsQuality = 1;
    settings_.targetFps = 144;
    settings_.fov = 75;
    settings_.showFps = true;
}

void Game::LoadSettings() {
    std::ifstream file(settingsPath_);
    if (!file) return;
    std::string key;
    bool loadedJump = false;
    bool loadedSneak = false;
    while (file >> key) {
        if (key == "master") file >> settings_.masterVolume;
        else if (key == "music") file >> settings_.musicVolume;
        else if (key == "sfx") file >> settings_.sfxVolume;
        else if (key == "quality") file >> settings_.graphicsQuality;
        else if (key == "fps") file >> settings_.targetFps;
        else if (key == "fov") file >> settings_.fov;
        else if (key == "showFps") file >> settings_.showFps;
        else if (key == "bind") {
            int action = 0, primary = 0, alt = 0;
            file >> action >> primary >> alt;
            if (action >= 0 && action < static_cast<int>(InputAction::Count)) {
                const InputAction inputAction = static_cast<InputAction>(action);
                if (inputAction == InputAction::Jump) loadedJump = true;
                if (inputAction == InputAction::Sneak) loadedSneak = true;
                if (KeyBinding* b = Binding(inputAction)) {
                    b->key = static_cast<KeyboardKey>(primary);
                    b->altKey = static_cast<KeyboardKey>(alt);
                }
            }
        }
    }
    if (!loadedJump) {
        if (KeyBinding* b = Binding(InputAction::Jump)) {
            b->key = KEY_SPACE;
            b->altKey = KEY_NULL;
        }
        if (KeyBinding* dash = Binding(InputAction::Dash); dash && (dash->key == KEY_SPACE || dash->altKey == KEY_SPACE)) {
            dash->key = KEY_LEFT_CONTROL;
            dash->altKey = KEY_RIGHT_CONTROL;
        }
    }
    if (!loadedSneak) {
        if (KeyBinding* b = Binding(InputAction::Sneak)) {
            b->key = KEY_LEFT_SHIFT;
            b->altKey = KEY_RIGHT_SHIFT;
        }
    }
}

void Game::SaveSettings() {
    std::ofstream file(settingsPath_, std::ios::trunc);
    if (!file) return;
    file << "master " << settings_.masterVolume << "\n";
    file << "music " << settings_.musicVolume << "\n";
    file << "sfx " << settings_.sfxVolume << "\n";
    file << "quality " << settings_.graphicsQuality << "\n";
    file << "fps " << settings_.targetFps << "\n";
    file << "fov " << settings_.fov << "\n";
    file << "showFps " << settings_.showFps << "\n";
    for (const auto& b : settings_.bindings) {
        file << "bind " << static_cast<int>(b.action) << " " << static_cast<int>(b.key) << " " << static_cast<int>(b.altKey) << "\n";
    }
}

void Game::ApplySettings() {
    settings_.masterVolume = std::max(0, std::min(100, settings_.masterVolume));
    settings_.musicVolume = std::max(0, std::min(100, settings_.musicVolume));
    settings_.sfxVolume = std::max(0, std::min(100, settings_.sfxVolume));
    settings_.graphicsQuality = std::max(0, std::min(3, settings_.graphicsQuality));
    settings_.targetFps = std::max(0, settings_.targetFps);
    settings_.fov = std::max(30, std::min(120, settings_.fov));
    camera_.fovy = static_cast<float>(settings_.fov);
    SetMasterVolume(settings_.masterVolume / 100.0f);
    SetTargetFPS(settings_.targetFps);
}

void Game::ApplyPendingSettings() {
    const bool borderlessChanged = pendingBorderlessWindowed_ != borderlessWindowed_;
    settings_.masterVolume = pendingSettings_.masterVolume;
    settings_.musicVolume = pendingSettings_.musicVolume;
    settings_.sfxVolume = pendingSettings_.sfxVolume;
    settings_.graphicsQuality = pendingSettings_.graphicsQuality;
    settings_.targetFps = pendingSettings_.targetFps;
    settings_.fov = pendingSettings_.fov;
    settings_.showFps = pendingSettings_.showFps;
    ApplySettings();
    if (borderlessChanged) ToggleBorderlessWindowedMode();
    pendingSettings_ = settings_;
    pendingBorderlessWindowed_ = borderlessWindowed_;
    SaveSettings();
    SetMessage("Paramètres appliqués.", 1.4f);
}

bool Game::HasPendingSettingsChanges() const {
    return pendingSettings_.masterVolume != settings_.masterVolume ||
           pendingSettings_.musicVolume != settings_.musicVolume ||
           pendingSettings_.sfxVolume != settings_.sfxVolume ||
           pendingSettings_.graphicsQuality != settings_.graphicsQuality ||
           pendingSettings_.targetFps != settings_.targetFps ||
           pendingSettings_.fov != settings_.fov ||
           pendingSettings_.showFps != settings_.showFps ||
           pendingBorderlessWindowed_ != borderlessWindowed_;
}

KeyBinding* Game::Binding(InputAction action) {
    for (auto& b : settings_.bindings) if (b.action == action) return &b;
    return nullptr;
}

const KeyBinding* Game::Binding(InputAction action) const {
    for (const auto& b : settings_.bindings) if (b.action == action) return &b;
    return nullptr;
}

bool Game::IsActionPressed(InputAction action) const {
    const KeyBinding* b = Binding(action);
    if (!b) return false;
    return (b->key != KEY_NULL && IsKeyPressed(b->key)) || (b->altKey != KEY_NULL && IsKeyPressed(b->altKey));
}

bool Game::IsActionDown(InputAction action) const {
    const KeyBinding* b = Binding(action);
    if (!b) return false;
    return (b->key != KEY_NULL && IsKeyDown(b->key)) || (b->altKey != KEY_NULL && IsKeyDown(b->altKey));
}

std::string Game::KeyLabel(KeyboardKey key) const {
    if (key == KEY_NULL) return "-";
    if (key >= KEY_A && key <= KEY_Z) return std::string(1, static_cast<char>('A' + (key - KEY_A)));
    if (key >= KEY_ZERO && key <= KEY_NINE) return std::string(1, static_cast<char>('0' + (key - KEY_ZERO)));
    switch (key) {
        case KEY_SPACE: return "Espace";
        case KEY_TAB: return "Tab";
        case KEY_ESCAPE: return "Échap";
        case KEY_ENTER: return "Entrée";
        case KEY_LEFT: return "Gauche";
        case KEY_RIGHT: return "Droite";
        case KEY_UP: return "Haut";
        case KEY_DOWN: return "Bas";
        case KEY_LEFT_SHIFT: return "Shift";
        case KEY_RIGHT_SHIFT: return "Shift droit";
        case KEY_LEFT_CONTROL: return "Ctrl";
        case KEY_RIGHT_CONTROL: return "Ctrl droit";
        default: return "Key " + std::to_string(static_cast<int>(key));
    }
}

void Game::EnsureSceneRenderTarget() {
    const int screenWidth = std::max(1, GetScreenWidth());
    const int screenHeight = std::max(1, GetScreenHeight());
    const float scale = std::min(1.0f, std::min(
        1600.0f / static_cast<float>(screenWidth),
        900.0f / static_cast<float>(screenHeight)));
    const int targetWidth = std::max(1, static_cast<int>(std::round(screenWidth * scale)));
    const int targetHeight = std::max(1, static_cast<int>(std::round(screenHeight * scale)));

    if (sceneTarget_.id != 0 &&
        sceneRenderWidth_ == targetWidth &&
        sceneRenderHeight_ == targetHeight) {
        return;
    }

    if (sceneTarget_.id != 0) UnloadRenderTexture(sceneTarget_);
    sceneTarget_ = LoadRenderTexture(targetWidth, targetHeight);
    SetTextureFilter(sceneTarget_.texture, TEXTURE_FILTER_BILINEAR);
    sceneRenderWidth_ = targetWidth;
    sceneRenderHeight_ = targetHeight;
}

void Game::RenderWorldToTarget() {
    const bool mainMenuBackground =
        mode_ == GameMode::MainMenu ||
        (mode_ == GameMode::Options && !optionsFromPause_ && previousMode_ == GameMode::MainMenu);
    if (mainMenuBackground || sceneTarget_.id == 0) return;

    BeginTextureMode(sceneTarget_);
    ClearBackground(Color{9, 9, 13, 255});
    BeginMode3D(camera_);
    if (mode_ == GameMode::Hub || previousMode_ == GameMode::Hub) DrawHub();
    if (mode_ == GameMode::Dungeon || previousMode_ == GameMode::Dungeon) {
        dungeon_.Draw(player_.position);
        boss_.Draw();
        for (const auto& p : projectiles_) if (p.alive) p.Draw();
    }
    if (!IsInGameplay()) DrawPlayer();
    else if (player_.castingReturn) {
        DrawCylinderWires({player_.position.x, 0.08f, player_.position.z}, 2.0f, 2.0f, 0.1f, 48, SKYBLUE);
    }
    EndMode3D();
    EndTextureMode();
}

void Game::Draw() {
    UI::BeginFrame();

    if (mode_ == GameMode::MainMenu || (mode_ == GameMode::Options && !optionsFromPause_ && previousMode_ == GameMode::MainMenu)) {
        DrawMainMenu();
        if (mode_ == GameMode::Options) {
            UI::BeginFrame();
            DrawOptionsMenu();
        }
        if (settings_.showFps) DrawFPS(GetScreenWidth() - 100, 12);
        return;
    }

    if (sceneTarget_.id != 0) {
        const Rectangle source {
            0.0f, 0.0f,
            static_cast<float>(sceneRenderWidth_),
            -static_cast<float>(sceneRenderHeight_)
        };
        const Rectangle destination {
            0.0f, 0.0f,
            static_cast<float>(GetScreenWidth()),
            static_cast<float>(GetScreenHeight())
        };
        DrawTexturePro(sceneTarget_.texture, source, destination, Vector2{}, 0.0f, WHITE);
    }

    const Room* room = mode_ == GameMode::Dungeon ? dungeon_.CurrentRoom(player_.position) : nullptr;
    UI::DrawHud(player_, save_.inventory, dungeon_.currentFloor, room, boss_, message_);
    if (IsInGameplay()) UI::DrawFirstPersonOverlay(player_, save_.inventory);
    UI::DrawHelp();

    if (mode_ == GameMode::ClassSelection) UI::DrawClassSelection(selectedIndex_);
    else if (mode_ == GameMode::WeaponShop) UI::DrawShopMenu("Marchand d'armes", weaponShop_, save_.inventory, selectedIndex_);
    else if (mode_ == GameMode::ItemShop) UI::DrawShopMenu("Marchand d'objets", itemShop_, save_.inventory, selectedIndex_);
    else if (mode_ == GameMode::Vendor) UI::DrawVendorMenu(vendor_, save_.inventory, selectedIndex_);
    else if (mode_ == GameMode::Crafting) UI::DrawCraftingMenu(crafting_, save_.inventory, selectedIndex_);
    else if (mode_ == GameMode::Upgrades) UI::DrawUpgradeMenu(upgradeStation_, save_.upgrades, save_.inventory, selectedIndex_);
    else if (mode_ == GameMode::Inventory) UI::DrawInventoryMenu(save_.inventory, selectedIndex_);
    else if (mode_ == GameMode::Chest) UI::DrawChestMenu(save_.inventory, selectedIndex_, chestSide_);
    else if (mode_ == GameMode::Options) {
        DrawOptionsMenu();
    } else if (mode_ == GameMode::Pause) {
        UI::DrawCenteredPanel("Menu pause", {
            (selectedIndex_ == 0 ? "> " : "  ") + std::string("Reprendre"),
            (selectedIndex_ == 1 ? "> " : "  ") + std::string("Options"),
            (selectedIndex_ == 2 ? "> " : "  ") + std::string("Sauvegarder"),
            (selectedIndex_ == 3 ? "> " : "  ") + std::string(previousMode_ == GameMode::Dungeon ? "Retour au sanctuaire" : "Retour au jeu"),
            (selectedIndex_ == 4 ? "> " : "  ") + std::string("Revenir au menu principal"),
            (selectedIndex_ == 5 ? "> " : "  ") + std::string("Sauvegarder et quitter"),
            "",
            "Entrée : valider | Échap : reprendre"
        });
    }

    if (settings_.showFps) DrawFPS(GetScreenWidth() - 100, 12);
}

void Game::DrawMainMenu() {
    const int w = GetScreenWidth();
    const int h = GetScreenHeight();
    DrawRectangle(0, 0, w, h, Color{7, 7, 11, 255});
    for (int i = 0; i < h; i += 6) {
        const unsigned char a = static_cast<unsigned char>(50 + (i * 80 / std::max(1, h)));
        DrawRectangle(0, i, w, 6, Color{20, 15, 22, a});
    }
    DrawTriangle(Vector2{w * 0.50f, h * 0.12f}, Vector2{w * 0.10f, h * 0.88f}, Vector2{w * 0.90f, h * 0.88f}, Color{37, 30, 32, 235});
    DrawTriangleLines(Vector2{w * 0.50f, h * 0.12f}, Vector2{w * 0.10f, h * 0.88f}, Vector2{w * 0.90f, h * 0.88f}, Color{216, 174, 82, 255});
    DrawCircle(w / 2, static_cast<int>(h * 0.38f), 110.0f, FadeColor(Color{74, 205, 196, 255}, 0.16f));
    DrawText("PYRAMID DUNGEON", w / 2 - MeasureText("PYRAMID DUNGEON", 54) / 2, static_cast<int>(h * 0.20f), 54, Color{232, 196, 86, 255});
    DrawText("Sanctuaire de l'Entrée", w / 2 - MeasureText("Sanctuaire de l'Entrée", 24) / 2, static_cast<int>(h * 0.29f), 24, RAYWHITE);
    UI::DrawCenteredPanel("Menu principal", {
        (selectedIndex_ == 0 ? "> " : "  ") + std::string("Entrer dans le sanctuaire"),
        (selectedIndex_ == 1 ? "> " : "  ") + std::string("Options"),
        (selectedIndex_ == 2 ? "> " : "  ") + std::string("Quitter"),
        "",
        "Clic gauche ou Entrée : valider"
    });
}

void Game::DrawOptionsMenu() {
    struct OptionRow {
        std::string label;
        std::string value;
        std::string description;
        bool slider = false;
        bool toggle = false;
    };

    const int screenW = GetScreenWidth();
    const int screenH = GetScreenHeight();
    const int margin = std::max(16, std::min(32, screenW / 60));
    const int titleH = screenH < 700 ? 58 : 68;
    const int tabH = screenH < 700 ? 44 : 52;
    const int rowH = screenH < 700 ? 42 : 50;
    const int innerW = screenW - margin * 2;
    const int gap = 14;
    const int leftW = std::max(520, static_cast<int>(innerW * 0.66f));
    const int rightW = innerW - leftW - gap;
    const int titleY = margin;
    const int tabsY = titleY + titleH + 12;
    const int contentY = tabsY + tabH + 10;
    const int footerH = 30;
    const int contentH = std::max(250, screenH - contentY - margin - footerH);
    const Rectangle leftPanel {
        static_cast<float>(margin), static_cast<float>(contentY),
        static_cast<float>(leftW), static_cast<float>(contentH)
    };
    const Rectangle rightPanel {
        static_cast<float>(margin + leftW + gap), static_cast<float>(contentY),
        static_cast<float>(rightW), static_cast<float>(contentH)
    };
    const Color background {8, 8, 10, 255};
    const Color panel {18, 16, 16, 252};
    const Color panelSoft {24, 20, 18, 252};
    const Color orange {226, 126, 48, 255};
    const Color orangeBright {244, 158, 67, 255};
    const Color orangeDark {104, 55, 24, 255};
    const Color stoneLine {65, 56, 50, 255};
    const Color rowBase {31, 28, 27, 255};
    const Color rowHover {57, 40, 29, 255};
    const Color selectedRow {6, 6, 7, 255};
    const Color ivory {235, 226, 211, 255};
    const Color muted {166, 151, 135, 255};

    DrawRectangle(0, 0, screenW, screenH, background);
    DrawRectangle(0, 0, screenW, 5, orange);
    DrawRectangle(margin, titleY, std::min(560, innerW), titleH, BLACK);
    DrawRectangleLines(margin, titleY, std::min(560, innerW), titleH, orangeDark);
    DrawText("PARAMÈTRES", margin + 24, titleY + (titleH - 34) / 2, 34, ivory);

    const char* tabLabels[] = {"COMMANDES", "AUDIO", "GRAPHISMES"};
    const int tabsW = std::min(leftW, 720);
    const int tabW = (tabsW - 8) / 3;
    const Vector2 mouse = GetMousePosition();
    for (int i = 0; i < 3; ++i) {
        Rectangle tab {
            static_cast<float>(margin + i * (tabW + 4)), static_cast<float>(tabsY),
            static_cast<float>(tabW), static_cast<float>(tabH)
        };
        const bool active = optionsSection_ == i;
        const bool focused = selectedIndex_ == i;
        const bool hovered = CheckCollisionPointRec(mouse, tab);
        DrawRectangleRec(tab, active ? Color{42, 31, 24, 255} : (hovered ? rowHover : Color{13, 12, 13, 255}));
        DrawRectangleLinesEx(tab, focused ? 2.0f : 1.0f, focused ? orangeBright : stoneLine);
        if (active) DrawRectangle(static_cast<int>(tab.x), static_cast<int>(tab.y + tab.height - 5), static_cast<int>(tab.width), 5, orange);
        const Color tabText = active ? orangeBright : ivory;
        const int fontSize = 21;
        const int textW = MeasureText(tabLabels[i], fontSize);
        DrawText(tabLabels[i], static_cast<int>(tab.x + (tab.width - textW) * 0.5f),
                 static_cast<int>(tab.y + (tab.height - fontSize) * 0.5f), fontSize, tabText);
        UI::AddMenuHitbox(i, tab);
    }

    std::vector<OptionRow> rows;
    std::string sectionTitle;
    if (optionsSection_ == 0) {
        sectionTitle = "Configuration clavier";
        for (size_t i = 0; i < settings_.bindings.size(); ++i) {
            const auto& binding = settings_.bindings[i];
            std::string keys = KeyLabel(binding.key);
            if (binding.altKey != KEY_NULL) keys += " / " + KeyLabel(binding.altKey);
            if (waitingForBindIndex_ == static_cast<int>(i)) keys = "APPUYEZ SUR UNE TOUCHE";
            rows.push_back({binding.name, keys, binding.description, false});
        }
        rows.push_back({"Commandes par défaut", "RÉINITIALISER", "Restaure toutes les commandes d'origine.", false});
        rows.push_back({"Retour", "", "Retourne au menu précédent.", false});
    } else if (optionsSection_ == 1) {
        sectionTitle = "Mixage audio";
        const std::string applyState = HasPendingSettingsChanges() ? "MODIFIÉ" : "À JOUR";
        rows = {
            {"Volume général", std::to_string(pendingSettings_.masterVolume) + "%", "Règle le volume global du jeu.", true},
            {"Musique", std::to_string(pendingSettings_.musicVolume) + "%", "Règle uniquement le volume de la musique.", true},
            {"Effets sonores", std::to_string(pendingSettings_.sfxVolume) + "%", "Règle les attaques, impacts et sons d'interface.", true},
            {"Appliquer", applyState, "Valide tous les changements audio et graphiques en attente.", false},
            {"Retour", "", "Retourne au menu précédent sans appliquer les changements en attente.", false}
        };
    } else {
        sectionTitle = "Affichage";
        const char* quality[] = {"Bas", "Moyen", "Élevé", "Ultra"};
        const std::string applyState = HasPendingSettingsChanges() ? "MODIFIÉ" : "À JOUR";
        const std::string fpsValue = editingFpsValue_
            ? (fpsValueBuffer_.empty() ? "_" : fpsValueBuffer_ + "_")
            : (pendingSettings_.targetFps == 0 ? "ILLIMITÉ" : std::to_string(pendingSettings_.targetFps) + " FPS");
        rows = {
            {"Qualité visuelle", quality[pendingSettings_.graphicsQuality], "Choisis Bas, Moyen, Élevé ou Ultra.", true},
            {"Limite d'images", fpsValue, "Glisse pour régler de 0 à 360, ou clique sur la valeur pour saisir un nombre exact. 0 signifie illimité.", true},
            {"Champ de vision", std::to_string(pendingSettings_.fov), "Règle librement le champ de vision entre 30 et 120.", true},
            {"Compteur FPS", pendingSettings_.showFps ? "AFFICHÉ" : "MASQUÉ", "Affiche le nombre d'images par seconde en haut à droite.", false, true},
            {"Plein écran fenêtré", pendingBorderlessWindowed_ ? "ACTIF" : "FENÊTRÉ",
             "Fenêtre sans bordure à la taille du moniteur. Le changement prend effet avec Appliquer.", false},
            {"Appliquer", applyState, "Valide d'un coup la limite FPS, le FOV, la qualité, l'affichage FPS et les volumes.", false},
            {"Retour", "", "Retourne au menu précédent sans appliquer les changements en attente.", false}
        };
    }

    DrawRectangleRec(leftPanel, panel);
    DrawRectangleLinesEx(leftPanel, 2.0f, orangeDark);
    const int sectionHeaderH = screenH < 700 ? 42 : 48;
    Rectangle sectionHeader {leftPanel.x, leftPanel.y, leftPanel.width, static_cast<float>(sectionHeaderH)};
    DrawRectangleRec(sectionHeader, Color{35, 30, 27, 255});
    DrawRectangleLinesEx(sectionHeader, 1.0f, stoneLine);
    DrawText(FitLabel(sectionTitle, 23, leftW - 36).c_str(), margin + 18,
             contentY + (sectionHeaderH - 23) / 2, 23, ivory);

    const int listTop = contentY + sectionHeaderH + 7;
    const int listBottom = contentY + contentH - 8;
    const int visibleRows = std::max(1, (listBottom - listTop) / rowH);
    const int maxScroll = std::max(0, static_cast<int>(rows.size()) - visibleRows);
    const int selectedLocal = selectedIndex_ >= 3 ? selectedIndex_ - 3 : -1;
    if (selectedLocal >= 0 && selectedLocal < static_cast<int>(rows.size())) {
        if (selectedLocal < optionsScroll_) optionsScroll_ = selectedLocal;
        if (selectedLocal >= optionsScroll_ + visibleRows) optionsScroll_ = selectedLocal - visibleRows + 1;
    }
    optionsScroll_ = std::max(0, std::min(maxScroll, optionsScroll_));

    const int rowWidth = leftW - (maxScroll > 0 ? 20 : 10);
    for (int visible = 0; visible < visibleRows; ++visible) {
        const int localIndex = optionsScroll_ + visible;
        if (localIndex >= static_cast<int>(rows.size())) break;
        const int menuIndex = 3 + localIndex;
        const OptionRow& row = rows[localIndex];
        Rectangle rowRect {
            leftPanel.x + 5.0f,
            static_cast<float>(listTop + visible * rowH),
            static_cast<float>(rowWidth),
            static_cast<float>(rowH - 4)
        };
        const bool selected = selectedIndex_ == menuIndex;
        const bool hovered = CheckCollisionPointRec(mouse, rowRect);
        const bool applyRow = row.label == "Appliquer";
        const bool pendingChanges = HasPendingSettingsChanges();
        const Color normalFill = applyRow
            ? (pendingChanges ? Color{85, 43, 20, 255} : Color{28, 27, 27, 255})
            : rowBase;
        DrawRectangleRec(rowRect, selected ? selectedRow : (hovered ? rowHover : normalFill));
        if (selected) {
            DrawRectangleLinesEx(rowRect, 3.0f, orange);
            DrawRectangleLinesEx(Rectangle{rowRect.x + 5.0f, rowRect.y + 5.0f, rowRect.width - 10.0f, rowRect.height - 10.0f},
                                 1.0f, orangeBright);
        } else {
            DrawRectangleLinesEx(rowRect, applyRow && pendingChanges ? 2.0f : 1.0f,
                                 hovered || (applyRow && pendingChanges) ? orangeDark : stoneLine);
        }

        const Color textColor = selected || (applyRow && pendingChanges) ? ivory : Color{218, 209, 197, 255};
        const Color valueColor = selected ? orangeBright : orange;
        const int fontSize = screenH < 700 ? 17 : 20;
        const Rectangle sliderRect = OptionsSliderRectangle(rowRect);
        const int labelAreaW = row.slider
            ? static_cast<int>(sliderRect.x - rowRect.x) - 34
            : rowWidth - std::max(150, rowWidth / 3) - 34;
        const std::string fittedLabel = FitLabel(row.label, fontSize, labelAreaW);
        DrawText(fittedLabel.c_str(), static_cast<int>(rowRect.x) + 18,
                 static_cast<int>(rowRect.y + (rowRect.height - fontSize) * 0.5f), fontSize, textColor);

        if (row.slider) {
            int sliderValue = 0;
            int minValue = 0;
            int maxValue = 0;
            bool stepped = false;
            int stepCount = 0;
            GetOptionsSliderValue(localIndex, sliderValue, minValue, maxValue, stepped, stepCount);
            const float ratio = maxValue > minValue
                ? Clamp01((sliderValue - minValue) / static_cast<float>(maxValue - minValue))
                : 0.0f;
            const float knobX = sliderRect.x + sliderRect.width * ratio;
            const int trackY = static_cast<int>(sliderRect.y);
            DrawRectangleRec(sliderRect, Color{11, 10, 11, 255});
            DrawRectangle(static_cast<int>(sliderRect.x), trackY,
                          static_cast<int>(sliderRect.width * ratio), static_cast<int>(sliderRect.height), orangeDark);
            if (stepped) {
                for (int step = 0; step < stepCount; ++step) {
                    const float tickRatio = stepCount > 1 ? step / static_cast<float>(stepCount - 1) : 0.0f;
                    const int tickX = static_cast<int>(sliderRect.x + sliderRect.width * tickRatio);
                    DrawLine(tickX, trackY - 3, tickX, trackY + 9,
                             tickRatio <= ratio + 0.001f ? orange : stoneLine);
                }
            }
            DrawCircle(static_cast<int>(knobX), trackY + 3, selected ? 9.0f : 8.0f, selected ? orangeBright : orange);
            DrawCircleLines(static_cast<int>(knobX), trackY + 3, selected ? 10.0f : 9.0f, selected ? ivory : orangeDark);

            const bool fpsValueField = optionsSection_ == 2 && localIndex == 1;
            const Rectangle valueRect = OptionsSliderValueRectangle(rowRect);
            const std::string fittedValue = FitLabel(row.value, fontSize, static_cast<int>(valueRect.width) - 10);
            const int valueW = MeasureText(fittedValue.c_str(), fontSize);
            if (fpsValueField) {
                DrawRectangleRec(valueRect, Color{12, 11, 12, 255});
                DrawRectangleLinesEx(valueRect, editingFpsValue_ ? 2.0f : 1.0f,
                                     editingFpsValue_ ? orangeBright : orangeDark);
                DrawText(fittedValue.c_str(),
                         static_cast<int>(valueRect.x + (valueRect.width - valueW) * 0.5f),
                         static_cast<int>(valueRect.y + (valueRect.height - fontSize) * 0.5f),
                         fontSize, valueColor);
            } else {
                DrawText(fittedValue.c_str(), static_cast<int>(rowRect.x + rowRect.width) - valueW - 14,
                         static_cast<int>(rowRect.y + (rowRect.height - fontSize) * 0.5f), fontSize, valueColor);
            }
        } else if (row.toggle) {
            const Rectangle toggleRect {
                rowRect.x + rowRect.width - 86.0f,
                rowRect.y + rowRect.height * 0.5f - 11.0f,
                66.0f, 22.0f
            };
            DrawRectangleRec(toggleRect, pendingSettings_.showFps ? orangeDark : Color{18, 17, 18, 255});
            DrawRectangleLinesEx(toggleRect, 1.0f, pendingSettings_.showFps ? orange : stoneLine);
            const float knobX = pendingSettings_.showFps ? toggleRect.x + toggleRect.width - 12.0f : toggleRect.x + 12.0f;
            DrawCircle(static_cast<int>(knobX), static_cast<int>(toggleRect.y + toggleRect.height * 0.5f),
                       8.0f, pendingSettings_.showFps ? orangeBright : muted);
        } else {
            const std::string fittedValue = FitLabel(row.value, fontSize, std::max(120, rowWidth / 3));
            const int valueW = MeasureText(fittedValue.c_str(), fontSize);
            DrawText(fittedValue.c_str(), static_cast<int>(rowRect.x + rowRect.width) - valueW - 18,
                     static_cast<int>(rowRect.y + (rowRect.height - fontSize) * 0.5f), fontSize, valueColor);
        }
        UI::AddMenuHitbox(menuIndex, rowRect);
    }

    if (maxScroll > 0) {
        Rectangle track {
            leftPanel.x + leftPanel.width - 10.0f, static_cast<float>(listTop),
            5.0f, static_cast<float>(visibleRows * rowH - 4)
        };
        DrawRectangleRec(track, Color{10, 9, 10, 255});
        DrawRectangleLinesEx(track, 1.0f, stoneLine);
        const float thumbH = std::max(34.0f, track.height * visibleRows / static_cast<float>(rows.size()));
        const float travel = track.height - thumbH;
        const float thumbY = track.y + (maxScroll > 0 ? travel * optionsScroll_ / maxScroll : 0.0f);
        DrawRectangleRec(Rectangle{track.x, thumbY, track.width, thumbH}, orange);
    }

    DrawRectangleRec(rightPanel, panelSoft);
    DrawRectangleLinesEx(rightPanel, 2.0f, orangeDark);
    const int descriptionHeaderH = sectionHeaderH;
    DrawRectangle(static_cast<int>(rightPanel.x), static_cast<int>(rightPanel.y),
                  static_cast<int>(rightPanel.width), descriptionHeaderH, Color{82, 43, 22, 255});
    DrawRectangleLinesEx(Rectangle{rightPanel.x, rightPanel.y, rightPanel.width, static_cast<float>(descriptionHeaderH)},
                         1.0f, orange);
    const char* descriptionTitle = "DESCRIPTION";
    const int descriptionTitleW = MeasureText(descriptionTitle, 22);
    DrawText(descriptionTitle, static_cast<int>(rightPanel.x + (rightPanel.width - descriptionTitleW) * 0.5f),
             static_cast<int>(rightPanel.y) + (descriptionHeaderH - 22) / 2, 22, ivory);

    std::string detailName = sectionTitle;
    std::string detailText = optionsSection_ == 0
        ? "Sélectionne une commande pour consulter son rôle ou modifier sa touche."
        : (optionsSection_ == 1
            ? "Sélectionne un canal puis clique ou fais glisser son curseur."
            : "Sélectionne un paramètre graphique puis clique ou fais glisser son curseur.");
    if (selectedLocal >= 0 && selectedLocal < static_cast<int>(rows.size())) {
        detailName = rows[selectedLocal].label;
        detailText = rows[selectedLocal].description;
        if (waitingForBindIndex_ >= 0 && optionsSection_ == 0) {
            detailText = "Appuie maintenant sur la nouvelle touche. Échap annule la modification.";
        }
    }

    const int detailX = static_cast<int>(rightPanel.x) + 24;
    const int detailY = static_cast<int>(rightPanel.y) + descriptionHeaderH + 24;
    DrawText(FitLabel(detailName, 25, rightW - 48).c_str(), detailX, detailY, 25, orangeBright);
    DrawRectangle(detailX, detailY + 38, std::max(40, rightW - 48), 2, orangeDark);
    DrawWrappedText(detailText,
                    Rectangle{static_cast<float>(detailX), static_cast<float>(detailY + 58),
                              static_cast<float>(rightW - 48), static_cast<float>(contentH - 165)},
                    19, 27, ivory);

    const std::string hint = waitingForBindIndex_ >= 0
        ? "Échap : annuler"
        : (editingFpsValue_
            ? "Chiffres : saisir   Entrée : valider   Échap : annuler"
            : "Clic/glisser : régler   Appliquer : valider les changements");
    DrawText(FitLabel(hint, 15, rightW - 48).c_str(), detailX,
             static_cast<int>(rightPanel.y + rightPanel.height) - 32, 15, muted);
    DrawText("Molette / Flèches : parcourir   Échap : retour", margin,
             screenH - margin - 20, 16, muted);
}

void Game::DrawHub() {
    DrawCube({0, -0.06f, 0}, 52.0f, 0.12f, 48.0f, Color{38, 35, 34, 255});
    DrawCubeWires({0, 0.02f, 0}, 52.0f, 0.12f, 48.0f, DARKGRAY);
    DrawCylinder({0, 0, 0}, 4.0f, 4.0f, 0.1f, 32, Color{47, 46, 58, 255});
    DrawCylinderWires({0, 0.08f, 0}, 4.2f, 4.2f, 0.1f, 32, GOLD);

    for (int i = -2; i <= 2; ++i) {
        DrawCylinder({-22.0f, 0, i * 8.0f}, 0.45f, 0.55f, 3.5f, 8, Color{82, 74, 63, 255});
        DrawCylinder({22.0f, 0, i * 8.0f}, 0.45f, 0.55f, 3.5f, 8, Color{82, 74, 63, 255});
        DrawSphere({-22.0f, 3.8f, i * 8.0f}, 0.35f, ORANGE);
        DrawSphere({22.0f, 3.8f, i * 8.0f}, 0.35f, ORANGE);
    }
    DrawHubZones();
}

void Game::DrawHubZones() const {
    for (const auto& z : hubZones_) {
        float d = DistanceXZ(player_.position, z.position);
        Color c = d < 2.6f ? WHITE : z.color;
        DrawCylinder({z.position.x, 0.04f, z.position.z}, 1.4f, 1.4f, 0.08f, 32, FadeColor(z.color, 0.65f));
        DrawCylinderWires({z.position.x, 0.1f, z.position.z}, 1.55f, 1.55f, 0.1f, 32, c);
        DrawCube({z.position.x, 0.75f, z.position.z}, 1.0f, 1.5f, 1.0f, FadeColor(z.color, 0.75f));
    }
}

void Game::DrawPlayer() const {
    Color c = player_.ClassColor();
    if (player_.invulnTimer > 0.0f) c = WHITE;
    DrawCapsule({player_.position.x, 0.1f, player_.position.z}, {player_.position.x, 1.8f, player_.position.z}, player_.radius, 12, 12, c);
    DrawSphere({player_.position.x + player_.facing.x * 0.45f, 1.55f, player_.position.z + player_.facing.z * 0.45f}, 0.18f, BLACK);
    DrawCircle3D({player_.position.x, 0.05f, player_.position.z}, player_.radius * 1.4f, {1, 0, 0}, 90, c);

    if (player_.castingReturn) {
        DrawCylinderWires({player_.position.x, 0.08f, player_.position.z}, 2.0f, 2.0f, 0.1f, 48, SKYBLUE);
    }
}
