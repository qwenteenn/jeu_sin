#include "Game.h"

Game::Game()
    : saveSystem_("save.txt"),
      weaponShop_(WeaponShopItems()),
      itemShop_(ConsumableShopItems()) {}

void Game::Run() {
    Init();
    while (!shouldQuit_ && !WindowShouldClose()) {
        const float dt = GetFrameTime();
        Update(dt);
        BeginDrawing();
        ClearBackground(Color{9, 9, 13, 255});
        Draw();
        EndDrawing();
    }
    SaveNow();
    CloseWindow();
}

void Game::Init() {
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pyramid Dungeon - C++ raylib");
    SetWindowMinSize(960, 540);
    SetExitKey(KEY_NULL);
    SetTargetFPS(144);
    DisableCursor();

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
    camera_.fovy = 60.0f;
    camera_.projection = CAMERA_PERSPECTIVE;

    hubZones_ = {
        {"class", "Choix de classe", {0, 0, -11}, SKYBLUE},
        {"weapons", "Marchand d'armes", {10, 0, -5}, ORANGE},
        {"items", "Marchand d'objets", {-10, 0, -5}, LIME},
        {"vendor", "Vente de loots", {10, 0, 6}, GOLD},
        {"craft", "Transformation", {-10, 0, 6}, PURPLE},
        {"upgrade", "Ameliorations", {0, 0, 8}, VIOLET},
        {"chest", "Coffre personnel", {-10, 0, -13}, BROWN},
        {"portal", "Portail du donjon", {0, 0, 16}, BLUE}
    };

    SetMessage("Bienvenue au Sanctuaire de l'Entree. E pour interagir avec les zones.", 4.0f);
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
        ToggleBorderlessWindowed();
        SetMessage("Mode plein ecran fenetre bascule.", 2.0f);
    }
    if (IsInGameplay() && IsKeyPressed(KEY_ESCAPE)) {
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

    const Vector3 eye {player_.position.x, player_.position.y + 1.55f, player_.position.z};
    const Vector3 view = ForwardView();
    camera_.position = eye;
    camera_.target = Vector3Add(eye, view);
    player_.facing = ForwardFlat();
}

void Game::UpdateMovement(float dt) {
    Vector3 forward = ForwardFlat();
    Vector3 right {std::cos(cameraYaw_), 0, -std::sin(cameraYaw_)};
    Vector3 move {0, 0, 0};

    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_Z)) move = Vector3Add(move, forward);
    if (IsKeyDown(KEY_S)) move = Vector3Subtract(move, forward);
    if (IsKeyDown(KEY_D)) move = Vector3Subtract(move, right);
    if (IsKeyDown(KEY_Q) || IsKeyDown(KEY_A)) move = Vector3Add(move, right);

    if (Vector3Length(move) > 0.01f) {
        move = Vector3Normalize(move);
        player_.facing = move;
        Vector3 next = AddScaled(player_.position, move, player_.speed * dt);
        if (IsWalkable(next)) player_.position = next;
    }

    if (IsKeyPressed(KEY_SPACE) && player_.dashCooldown <= 0.0f) {
        Vector3 dir = Vector3Length(move) > 0.01f ? move : ForwardFlat();
        float dashDistance = player_.classType == ClassType::Monk ? 5.2f : 3.1f;
        if (player_.classType == ClassType::Monk && player_.SpendResource(12.0f)) dashDistance = 6.2f;
        Vector3 next = AddScaled(player_.position, dir, dashDistance);
        if (IsWalkable(next)) player_.position = next;
        player_.dashCooldown = player_.classType == ClassType::Monk ? 0.55f : 1.0f;
        player_.invulnTimer = 0.22f;
        if (player_.classType == ClassType::Monk) player_.GainResource(8.0f);
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
    if (IsKeyPressed(KEY_E)) TryInteractHub();
    if (IsKeyPressed(KEY_TAB)) { previousMode_ = mode_; ChangeMode(GameMode::Inventory); }
    if (IsKeyPressed(KEY_ONE)) EnterDungeon(1);
    if (IsKeyPressed(KEY_TWO) && save_.maxUnlockedFloor >= 2) EnterDungeon(2);
    if (IsKeyPressed(KEY_THREE) && save_.maxUnlockedFloor >= 3) EnterDungeon(3);
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
    player_.hp = player_.maxHp;
    player_.resource = player_.maxResource;
    mode_ = GameMode::Dungeon;
    DisableCursor();
    mouseCaptured_ = true;
    SetMessage("Etage " + std::to_string(floor) + " : descente dans la pyramide.", 3.0f);
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
        ReturnToHub("Mort dans le donjon. Retour au sanctuaire avec penalite.", true);
    }
    if (IsKeyPressed(KEY_TAB)) { previousMode_ = mode_; ChangeMode(GameMode::Inventory); }
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
        SetMessage("Explosion elementaire.", 1.2f);
    } else if (player_.classType == ClassType::Necromancer) {
        if (!player_.SpendResource(35.0f)) { SetMessage("Pas assez d'energie d'ame.", 1.5f); return; }
        dungeon_.enemies.emplace_back(EnemyType::SummonedSkeleton, AddScaled(player_.position, player_.facing, 1.7f), dungeon_.currentFloor, true);
        float bonus = save_.upgrades.Level("necromancy") * 6.0f;
        dungeon_.enemies.back().hp += bonus;
        dungeon_.enemies.back().maxHp += bonus;
        player_.secondaryCooldown = 8.0f;
        SetMessage("Squelette invoque.", 1.5f);
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
    if (IsKeyPressed(KEY_A)) {
        if (save_.inventory.Remove("potion_health", 1)) {
            player_.Heal(45.0f);
            SetMessage("Potion de soin utilisee.", 1.5f);
            SaveNow();
        } else SetMessage("Aucune potion de soin.", 1.5f);
    }
    if (mode_ == GameMode::Dungeon && IsKeyPressed(KEY_R)) UseReturnItem();
}

void Game::UseReturnItem() {
    if (boss_.alive) {
        SetMessage("Impossible d'utiliser un retour pendant le boss. Evidemment, ce serait trop simple.", 2.5f);
        return;
    }
    if (save_.inventory.Remove("stable_return_crystal", 1)) {
        ReturnToHub("Cristal stable utilise : retour instantane.", false);
        return;
    }
    if (save_.inventory.Count("return_stone") <= 0) {
        SetMessage("Aucune Pierre de Retour.", 1.8f);
        return;
    }
    if (!player_.castingReturn) {
        player_.castingReturn = true;
        player_.returnCastTimer = 3.0f;
        SetMessage("Incantation de retour : 3 secondes. Ne te fais pas frapper, concept revolutionnaire.", 3.0f);
    }
}

void Game::TryInteractDungeon() {
    Room* room = dungeon_.CurrentRoom(player_.position);
    if (!room) return;

    if (player_.castingReturn) {
        player_.returnCastTimer -= GetFrameTime();
        if (player_.returnCastTimer <= 0.0f) {
            if (save_.inventory.Remove("return_stone", 1)) ReturnToHub("Pierre de Retour utilisee.", false);
            player_.castingReturn = false;
        }
    }

    if (!IsKeyPressed(KEY_E)) return;

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
            SetMessage("Le Cercle de Descente est verrouille. Nettoie l'etage avant de supplier le sol.", 2.5f);
            return;
        }
        if (dungeon_.currentFloor == 2 && save_.inventory.Count("ancient_key") <= 0) {
            SetMessage("Une Cle ancienne est necessaire pour ouvrir la descente.", 2.5f);
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
    SetMessage("Le Gardien de la Pyramide se reveille.", 3.0f);
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
    dungeon_.enemies.clear();
    dungeon_.loot.clear();
    projectiles_.clear();
    boss_ = Boss{};
    SaveNow();
    SetMessage(reason, 4.0f);
}

void Game::UpdateMenu(float dt) {
    (void)dt;
    if (IsKeyPressed(KEY_ESCAPE) || (mode_ == GameMode::Inventory && IsKeyPressed(KEY_TAB))) {
        ChangeMode(previousMode_);
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

    HandleMenuMouseInput();
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

void Game::ActivateMenuSelection(int action) {
    if (mode_ == GameMode::ClassSelection) {
        ClassType t = static_cast<ClassType>(selectedIndex_);
        save_.classType = t;
        player_.ChooseClass(t, save_.upgrades, save_.inventory);
        SaveNow();
        SetMessage("Classe choisie : " + ClassName(t), 2.0f);
        ChangeMode(GameMode::Hub);
    } else if (mode_ == GameMode::WeaponShop) {
        if (weaponShop_.Buy(selectedIndex_, save_.inventory)) { SaveNow(); SetMessage("Achat effectue.", 1.5f); }
        else SetMessage("Pas assez d'or ou achat impossible.", 1.8f);
    } else if (mode_ == GameMode::ItemShop) {
        if (itemShop_.Buy(selectedIndex_, save_.inventory)) { SaveNow(); SetMessage("Achat effectue.", 1.5f); }
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
            SaveNow(); SetMessage("Transformation effectuee.", 1.5f);
        } else SetMessage("Ressources insuffisantes.", 1.6f);
    } else if (mode_ == GameMode::Upgrades) {
        const auto& ups = upgradeStation_.Upgrades();
        if (selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(ups.size()) && upgradeStation_.Buy(ups[selectedIndex_], save_.upgrades, save_.inventory)) {
            player_.ApplyUpgrades(save_.upgrades, save_.inventory);
            SaveNow(); SetMessage("Amelioration achetee.", 1.5f);
        } else SetMessage("Pas assez de ressources pour ameliorer.", 1.8f);
    } else if (mode_ == GameMode::Inventory) {
        auto items = UI::InventoryDisplayItems(save_.inventory);
        if (selectedIndex_ < static_cast<int>(items.size())) {
            if (save_.inventory.EquipWeapon(items[selectedIndex_].first)) {
                player_.ApplyUpgrades(save_.upgrades, save_.inventory);
                SaveNow(); SetMessage("Arme equipee.", 1.2f);
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
                if (ok) { SaveNow(); SetMessage(chestSide_ ? "Objet retire du coffre." : "Objet depose au coffre.", 1.2f); }
            }
        }
    } else if (mode_ == GameMode::Pause) {
        if (selectedIndex_ == 0) {
            ChangeMode(previousMode_);
        } else if (selectedIndex_ == 1) {
            SaveNow();
            SetMessage("Partie sauvegardee.", 1.5f);
        } else if (selectedIndex_ == 2) {
            if (previousMode_ == GameMode::Dungeon) ReturnToHub("Retour au sanctuaire depuis le menu pause.", false);
            else ChangeMode(previousMode_);
        } else if (selectedIndex_ == 3) {
            SaveNow();
            shouldQuit_ = true;
        }
    }
}

int Game::MaxMenuIndex() const {
    switch (mode_) {
        case GameMode::ClassSelection: return 3;
        case GameMode::WeaponShop: return std::max(0, static_cast<int>(weaponShop_.Items().size()) - 1);
        case GameMode::ItemShop: return std::max(0, static_cast<int>(itemShop_.Items().size()) - 1);
        case GameMode::Vendor: return std::max(0, static_cast<int>(vendor_.SellableIds(save_.inventory).size()) - 1);
        case GameMode::Crafting: return std::max(0, static_cast<int>(crafting_.Recipes().size()) - 1);
        case GameMode::Upgrades: return std::max(0, static_cast<int>(upgradeStation_.Upgrades().size()) - 1);
        case GameMode::Inventory: return std::max(0, static_cast<int>(UI::InventoryDisplayItems(save_.inventory).size()) - 1);
        case GameMode::Chest: return std::max(0, static_cast<int>((chestSide_ ? save_.inventory.SortedChest() : save_.inventory.SortedItems()).size()) - 1);
        case GameMode::Pause: return 3;
        default: return 0;
    }
}

void Game::ChangeMode(GameMode mode) {
    selectedIndex_ = 0;
    mode_ = mode;
    if (IsInGameplay()) {
        DisableCursor();
        mouseCaptured_ = true;
    } else {
        EnableCursor();
        mouseCaptured_ = false;
    }
}

void Game::SaveNow() {
    save_.classType = player_.classType;
    saveSystem_.Save(save_);
}

void Game::SetMessage(const std::string& msg, float duration) {
    message_ = msg;
    messageTimer_ = duration;
}

void Game::Draw() {
    UI::BeginFrame();

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
    else if (mode_ == GameMode::Pause) {
        UI::DrawCenteredPanel("Menu pause", {
            (selectedIndex_ == 0 ? "> " : "  ") + std::string("Reprendre"),
            (selectedIndex_ == 1 ? "> " : "  ") + std::string("Sauvegarder"),
            (selectedIndex_ == 2 ? "> " : "  ") + std::string(previousMode_ == GameMode::Dungeon ? "Retour au sanctuaire" : "Retour au jeu"),
            (selectedIndex_ == 3 ? "> " : "  ") + std::string("Sauvegarder et quitter"),
            "",
            "Entree : valider | Echap : reprendre"
        });
    }

    DrawFPS(GetScreenWidth() - 100, 12);
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
