#include "UI.h"

namespace {
struct MenuHitbox {
    int index = -1;
    int action = UI::ACTION_PRIMARY;
    Rectangle rect {};
};

std::vector<MenuHitbox> g_menuHitboxes;

struct UiMetrics {
    int screenW = 1600;
    int screenH = 900;
    int margin = 20;
};

UiMetrics Metrics() {
    UiMetrics m;
    m.screenW = std::max(640, GetScreenWidth());
    m.screenH = std::max(420, GetScreenHeight());
    m.margin = std::max(14, std::min(28, m.screenW / 70));
    return m;
}

Color StonePanel() { return Color{12, 11, 15, 236}; }
Color StoneSoft() { return Color{27, 25, 31, 228}; }
Color GoldLine() { return Color{214, 172, 78, 255}; }
Color GoldSoft() { return Color{135, 100, 43, 255}; }
Color Turquoise() { return Color{74, 205, 196, 255}; }
Color Ink() { return Color{7, 7, 10, 180}; }

std::string ItemNameOrId(const std::string& id) {
    const ItemDef* def = GetItemDef(id);
    return def ? def->name : id;
}

std::string CategoryLabel(const std::string& category) {
    if (category == "weapon") return "Armes";
    if (category == "consumable") return "Consommables";
    if (category == "material") return "Materiaux";
    if (category == "loot") return "Loot";
    if (category == "relic") return "Reliques";
    return "Autres";
}

int CategoryRank(const ItemDef* def) {
    if (!def) return 99;
    if (def->category == "weapon") return 0;
    if (def->category == "consumable") return 1;
    if (def->category == "material") return 2;
    if (def->category == "loot") return 3;
    if (def->category == "relic") return 4;
    return 5;
}

Color CategoryColor(const ItemDef* def) {
    if (!def) return LIGHTGRAY;
    if (def->rare) return Color{232, 196, 86, 255};
    if (def->category == "weapon") return Color{231, 112, 86, 255};
    if (def->category == "armor") return Color{167, 173, 190, 255};
    if (def->category == "consumable") return Color{82, 201, 126, 255};
    if (def->category == "material") return Color{103, 190, 219, 255};
    if (def->category == "loot") return Color{184, 138, 91, 255};
    if (def->category == "relic") return Color{188, 118, 230, 255};
    if (def->category == "key") return Color{238, 196, 91, 255};
    return LIGHTGRAY;
}

std::string FitText(const std::string& text, int fontSize, int maxWidth) {
    if (MeasureText(text.c_str(), fontSize) <= maxWidth) return text;
    std::string out = text;
    while (!out.empty() && MeasureText((out + "...").c_str(), fontSize) > maxWidth) out.pop_back();
    return out.empty() ? std::string("...") : out + "...";
}

void DrawTextFit(const std::string& text, int x, int y, int fontSize, Color color, int maxWidth) {
    DrawText(FitText(text, fontSize, maxWidth).c_str(), x, y, fontSize, color);
}

void DrawPanel(Rectangle r, Color fill = StonePanel(), Color line = GoldLine()) {
    DrawRectangleRec(r, fill);
    DrawRectangleLinesEx(r, 2.0f, line);
    DrawRectangleLinesEx(Rectangle{r.x + 4, r.y + 4, r.width - 8, r.height - 8}, 1.0f, FadeColor(line, 0.32f));
}

void DrawHeader(const std::string& title, Rectangle r) {
    DrawRectangleRec(r, Color{34, 28, 34, 245});
    DrawRectangleLinesEx(r, 1.0f, GoldSoft());
    DrawTextFit(title, static_cast<int>(r.x) + 16, static_cast<int>(r.y) + 9, 22, GoldLine(), static_cast<int>(r.width) - 32);
}

void DrawItemIcon(const ItemDef* def, int cx, int cy, int size, bool equipped) {
    Color c = CategoryColor(def);
    if (!def) {
        DrawCircle(cx, cy, size / 5.0f, LIGHTGRAY);
        return;
    }

    if (def->category == "weapon") {
        DrawRectangle(cx - 4, cy - size / 3, 8, size * 2 / 3, c);
        DrawTriangle(Vector2{static_cast<float>(cx), static_cast<float>(cy - size / 2)},
                     Vector2{static_cast<float>(cx - 12), static_cast<float>(cy - size / 4)},
                     Vector2{static_cast<float>(cx + 12), static_cast<float>(cy - size / 4)}, c);
        DrawRectangle(cx - 16, cy + size / 5, 32, 5, GoldLine());
    } else if (def->category == "consumable") {
        DrawRectangle(cx - size / 5, cy - size / 3, size * 2 / 5, size * 2 / 3, c);
        DrawRectangle(cx - size / 7, cy - size / 2, size * 2 / 7, size / 6, RAYWHITE);
        DrawRectangleLines(cx - size / 5, cy - size / 3, size * 2 / 5, size * 2 / 3, WHITE);
    } else if (def->category == "material") {
        DrawPoly(Vector2{static_cast<float>(cx), static_cast<float>(cy)}, 6, size / 3.0f, 30.0f, c);
        DrawPolyLines(Vector2{static_cast<float>(cx), static_cast<float>(cy)}, 6, size / 3.0f, 30.0f, WHITE);
    } else if (def->category == "relic") {
        DrawCircle(cx, cy, size / 3.2f, c);
        DrawCircleLines(cx, cy, size / 2.8f, GoldLine());
        DrawRectangle(cx - 3, cy - size / 3, 6, size * 2 / 3, RAYWHITE);
    } else if (def->category == "armor") {
        DrawTriangle(Vector2{static_cast<float>(cx), static_cast<float>(cy + size / 3)},
                     Vector2{static_cast<float>(cx - size / 3), static_cast<float>(cy - size / 4)},
                     Vector2{static_cast<float>(cx + size / 3), static_cast<float>(cy - size / 4)}, c);
        DrawTriangleLines(Vector2{static_cast<float>(cx), static_cast<float>(cy + size / 3)},
                          Vector2{static_cast<float>(cx - size / 3), static_cast<float>(cy - size / 4)},
                          Vector2{static_cast<float>(cx + size / 3), static_cast<float>(cy - size / 4)}, WHITE);
    } else if (def->category == "key") {
        DrawCircleLines(cx - 8, cy, size / 6.0f, c);
        DrawRectangle(cx, cy - 3, size / 2, 6, c);
        DrawRectangle(cx + size / 3, cy + 3, 5, 9, c);
    } else {
        DrawCircle(cx, cy, size / 3.0f, c);
        DrawCircleLines(cx, cy, size / 3.0f, WHITE);
    }

    if (equipped) {
        DrawCircle(cx + size / 3, cy - size / 3, 10.0f, GoldLine());
        DrawText("E", cx + size / 3 - 5, cy - size / 3 - 7, 14, BLACK);
    }
}

int CountCategory(const Inventory& inventory, const std::string& category) {
    int total = 0;
    for (const auto& [id, amount] : inventory.items) {
        const ItemDef* def = GetItemDef(id);
        if (!def) continue;
        const bool other = category == "other" && def->category != "weapon" && def->category != "consumable" &&
                           def->category != "material" && def->category != "loot" && def->category != "relic";
        if (def->category == category || other) total += amount;
    }
    return total;
}
}

void UI::BeginFrame() {
    g_menuHitboxes.clear();
}

void UI::AddMenuHitbox(int index, Rectangle rect, int action) {
    g_menuHitboxes.push_back(MenuHitbox{index, action, rect});
}

int UI::HoveredMenuIndex() {
    const Vector2 mouse = GetMousePosition();
    for (auto it = g_menuHitboxes.rbegin(); it != g_menuHitboxes.rend(); ++it) {
        if (CheckCollisionPointRec(mouse, it->rect)) return it->index;
    }
    return -1;
}

int UI::HoveredMenuAction() {
    const Vector2 mouse = GetMousePosition();
    for (auto it = g_menuHitboxes.rbegin(); it != g_menuHitboxes.rend(); ++it) {
        if (CheckCollisionPointRec(mouse, it->rect)) return it->action;
    }
    return ACTION_PRIMARY;
}

bool UI::WasMenuItemClicked() {
    return HoveredMenuIndex() >= 0 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

void UI::DrawBar(int x, int y, int w, int h, float pct, Color fill, const std::string& label) {
    Rectangle r{static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h)};
    DrawRectangleRec(r, Color{15, 14, 18, 225});
    DrawRectangle(x + 2, y + 2, static_cast<int>((w - 4) * Clamp01(pct)), h - 4, fill);
    DrawRectangleLinesEx(r, 1.5f, GoldSoft());
    DrawTextFit(label, x + 9, y + std::max(4, (h - 18) / 2), 18, WHITE, w - 18);
}

void UI::DrawHud(const Player& player, const Inventory& inventory, int floor, const Room* room, const Boss& boss, const std::string& message) {
    const UiMetrics m = Metrics();
    const int hudW = std::min(390, std::max(300, m.screenW / 4));
    const int x = m.margin;
    const int y = m.margin;

    DrawPanel(Rectangle{static_cast<float>(x), static_cast<float>(y), static_cast<float>(hudW), 174.0f}, Color{10, 10, 14, 210}, GoldSoft());
    DrawBar(x + 12, y + 12, hudW - 24, 24, player.hp / player.maxHp, Color{190, 46, 50, 255},
            "Vie " + std::to_string(static_cast<int>(player.hp)) + "/" + std::to_string(static_cast<int>(player.maxHp)));

    if (player.maxResource > 0.0f) {
        Color c = player.classType == ClassType::Necromancer ? Color{163, 92, 218, 255} : (player.classType == ClassType::Monk ? GoldLine() : Turquoise());
        std::string res = player.classType == ClassType::Necromancer ? "Ame" : (player.classType == ClassType::Monk ? "Ki" : "Mana");
        DrawBar(x + 12, y + 42, hudW - 24, 22, player.resource / player.maxResource, c,
                res + " " + std::to_string(static_cast<int>(player.resource)) + "/" + std::to_string(static_cast<int>(player.maxResource)));
    }

    DrawTextFit("Classe : " + ClassName(player.classType), x + 14, y + 76, 17, WHITE, hudW - 28);
    DrawText(("Or : " + std::to_string(inventory.gold)).c_str(), x + 14, y + 99, 17, GoldLine());
    DrawText(("Potions : " + std::to_string(inventory.Count("potion_health"))).c_str(), x + hudW / 2, y + 99, 17, Color{105, 224, 138, 255});
    DrawText(("Retour : " + std::to_string(inventory.Count("return_stone"))).c_str(), x + 14, y + 122, 17, Turquoise());
    DrawText(("Etage : " + std::to_string(floor)).c_str(), x + hudW / 2, y + 122, 17, RAYWHITE);
    if (room) DrawTextFit(room->Name(), x + 14, y + 145, 16, LIGHTGRAY, hudW - 28);

    if (boss.alive) {
        const int bossW = std::min(680, m.screenW - m.margin * 2);
        DrawBar(m.screenW / 2 - bossW / 2, m.margin, bossW, 28, boss.hp / boss.maxHp, MAROON, "Gardien de la Pyramide");
    }

    if (!message.empty()) {
        const int msgW = std::min(860, m.screenW - m.margin * 2);
        const int msgX = m.screenW / 2 - msgW / 2;
        const int msgY = m.screenH - 86;
        DrawPanel(Rectangle{static_cast<float>(msgX), static_cast<float>(msgY), static_cast<float>(msgW), 42.0f}, Color{12, 12, 16, 218}, GoldSoft());
        DrawTextFit(message, msgX + 18, msgY + 12, 19, WHITE, msgW - 36);
    }
}

void UI::DrawFirstPersonOverlay(const Player& player, const Inventory& inventory) {
    const UiMetrics m = Metrics();
    const int cx = m.screenW / 2;
    const int cy = m.screenH / 2;

    DrawCircleLines(cx, cy, 7.0f, FadeColor(WHITE, 0.72f));
    DrawLine(cx - 18, cy, cx - 8, cy, FadeColor(GoldLine(), 0.82f));
    DrawLine(cx + 8, cy, cx + 18, cy, FadeColor(GoldLine(), 0.82f));
    DrawLine(cx, cy - 18, cx, cy - 8, FadeColor(GoldLine(), 0.82f));
    DrawLine(cx, cy + 8, cx, cy + 18, FadeColor(GoldLine(), 0.82f));
    DrawPixel(cx, cy, WHITE);

    const ItemDef* weapon = GetItemDef(inventory.equippedWeapon);
    const std::string weaponId = weapon ? weapon->id : "";
    const int baseX = m.screenW - std::max(250, m.screenW / 5);
    const int baseY = m.screenH - 150;
    Color classColor = player.ClassColor();
    Color metal = weapon && weapon->rare ? GoldLine() : Color{178, 174, 165, 255};
    Color grip = Color{76, 49, 34, 255};

    DrawRectangle(baseX + 74, baseY + 62, 112, 70, FadeColor(Color{52, 34, 30, 255}, 0.92f));
    DrawCircle(baseX + 82, baseY + 66, 28.0f, FadeColor(classColor, 0.85f));

    if (weaponId.find("staff") != std::string::npos || weaponId.find("mage") != std::string::npos) {
        DrawLineEx(Vector2{static_cast<float>(baseX + 104), static_cast<float>(baseY + 104)},
                   Vector2{static_cast<float>(baseX + 212), static_cast<float>(baseY + 26)}, 10.0f, grip);
        DrawCircle(baseX + 222, baseY + 18, 18.0f, FadeColor(Turquoise(), 0.85f));
        DrawCircleLines(baseX + 222, baseY + 18, 22.0f, GoldLine());
    } else if (weaponId.find("grimoire") != std::string::npos) {
        DrawRectangle(baseX + 126, baseY + 44, 92, 68, FadeColor(Color{72, 38, 96, 255}, 0.95f));
        DrawRectangleLines(baseX + 126, baseY + 44, 92, 68, GoldLine());
        DrawLine(baseX + 172, baseY + 50, baseX + 172, baseY + 106, FadeColor(WHITE, 0.45f));
        DrawCircle(baseX + 215, baseY + 46, 9.0f, FadeColor(PURPLE, 0.9f));
    } else if (weaponId.find("gloves") != std::string::npos) {
        DrawCircle(baseX + 166, baseY + 76, 33.0f, FadeColor(GoldLine(), 0.85f));
        DrawRectangle(baseX + 136, baseY + 80, 68, 28, FadeColor(Color{95, 58, 38, 255}, 0.95f));
        DrawLineEx(Vector2{static_cast<float>(baseX + 140), static_cast<float>(baseY + 74)},
                   Vector2{static_cast<float>(baseX + 194), static_cast<float>(baseY + 74)}, 5.0f, WHITE);
    } else if (weaponId.find("axe") != std::string::npos) {
        DrawLineEx(Vector2{static_cast<float>(baseX + 110), static_cast<float>(baseY + 120)},
                   Vector2{static_cast<float>(baseX + 202), static_cast<float>(baseY + 36)}, 11.0f, grip);
        DrawRectangle(baseX + 190, baseY + 18, 38, 52, metal);
        DrawTriangle(Vector2{static_cast<float>(baseX + 228), static_cast<float>(baseY + 18)},
                     Vector2{static_cast<float>(baseX + 256), static_cast<float>(baseY + 48)},
                     Vector2{static_cast<float>(baseX + 228), static_cast<float>(baseY + 70)}, metal);
    } else {
        DrawLineEx(Vector2{static_cast<float>(baseX + 118), static_cast<float>(baseY + 118)},
                   Vector2{static_cast<float>(baseX + 220), static_cast<float>(baseY + 28)}, 9.0f, grip);
        DrawLineEx(Vector2{static_cast<float>(baseX + 154), static_cast<float>(baseY + 82)},
                   Vector2{static_cast<float>(baseX + 236), static_cast<float>(baseY + 8)}, 7.0f, metal);
        DrawTriangle(Vector2{static_cast<float>(baseX + 238), static_cast<float>(baseY + 6)},
                     Vector2{static_cast<float>(baseX + 226), static_cast<float>(baseY + 28)},
                     Vector2{static_cast<float>(baseX + 250), static_cast<float>(baseY + 28)}, metal);
        DrawLineEx(Vector2{static_cast<float>(baseX + 148), static_cast<float>(baseY + 90)},
                   Vector2{static_cast<float>(baseX + 180), static_cast<float>(baseY + 110)}, 7.0f, GoldLine());
    }
}

void UI::DrawHelp() {
    const UiMetrics m = Metrics();
    const char* text = "ZQSD: bouger | Souris: regard/menu | Clic gauche: attaque/valider | Clic droit: competence | Espace: dash | E: interagir | A: potion | R: retour | TAB: inventaire | F11: plein ecran fenetre | ESC: pause";
    DrawRectangle(0, m.screenH - 30, m.screenW, 30, Color{0, 0, 0, 175});
    DrawTextFit(text, 16, m.screenH - 23, 16, LIGHTGRAY, m.screenW - 32);
}

void UI::DrawClassSelection(int selected) {
    std::vector<std::string> lines = {
        (selected == 0 ? "> " : "  ") + std::string("Mage elementaire - projectiles, explosion, mana"),
        (selected == 1 ? "> " : "  ") + std::string("Necromancien - trait sombre, invocation, energie d'ame"),
        (selected == 2 ? "> " : "  ") + std::string("Combattant - melee, charge, grosse defense"),
        (selected == 3 ? "> " : "  ") + std::string("Moine - combos rapides, dash spirituel, ki"),
        "",
        "Entree pour choisir, Echap pour fermer"
    };
    DrawCenteredPanel("Choix de classe", lines);
}

void UI::DrawShopMenu(const std::string& title, const Shop& shop, const Inventory& inventory, int selected) {
    std::vector<std::string> lines;
    lines.push_back("Or disponible : " + std::to_string(inventory.gold));
    lines.push_back("Entree : acheter | Echap : quitter");
    lines.push_back("");
    const auto& items = shop.Items();
    for (size_t i = 0; i < items.size(); ++i) {
        const ItemDef* def = GetItemDef(items[i]);
        if (!def) continue;
        lines.push_back((static_cast<int>(i) == selected ? "> " : "  ") + def->name + " - " + std::to_string(def->buyPrice) + " or");
    }
    DrawCenteredPanel(title, lines);
}

void UI::DrawVendorMenu(const Vendor& vendor, const Inventory& inventory, int selected) {
    std::vector<std::string> lines;
    lines.push_back("Or : " + std::to_string(inventory.gold));
    lines.push_back("Entree : vendre 1 | V : vendre communs auto | Echap : quitter");
    lines.push_back("");
    auto ids = vendor.SellableIds(inventory);
    if (ids.empty()) lines.push_back("Aucun loot vendable.");
    for (size_t i = 0; i < ids.size(); ++i) {
        const ItemDef* def = GetItemDef(ids[i]);
        if (!def) continue;
        lines.push_back((static_cast<int>(i) == selected ? "> " : "  ") + def->name + " x" + std::to_string(inventory.Count(ids[i])) + " - " + std::to_string(def->sellPrice) + " or/u" + (def->autoSell ? " [auto]" : " [rare]"));
    }
    DrawCenteredPanel("Vente de loots", lines);
    const UiMetrics m = Metrics();
    const int w = std::min(980, m.screenW - m.margin * 2);
    const int h = std::min(std::min(760, m.screenH - m.margin * 2), 120 + static_cast<int>(lines.size()) * 28);
    const int x = m.screenW / 2 - w / 2;
    const int y = m.screenH / 2 - h / 2;
    AddMenuHitbox(0, Rectangle{static_cast<float>(x + w - 230), static_cast<float>(y + 96), 190.0f, 26.0f}, ACTION_VENDOR_SELL_ALL);
    DrawRectangle(x + w - 230, y + 96, 190, 26, FadeColor(GoldSoft(), 0.45f));
    DrawRectangleLines(x + w - 230, y + 96, 190, 26, GoldLine());
    DrawTextFit("Vendre communs", x + w - 218, y + 101, 15, WHITE, 166);
}

void UI::DrawCraftingMenu(const CraftingStation& station, const Inventory& inventory, int selected) {
    std::vector<std::string> lines;
    lines.push_back("Entree : transformer | Echap : quitter");
    lines.push_back("");
    const auto& recipes = station.Recipes();
    for (size_t i = 0; i < recipes.size(); ++i) {
        const auto& r = recipes[i];
        std::string line = (static_cast<int>(i) == selected ? "> " : "  ") + r.name + " | Cout: " + std::to_string(r.goldCost) + " or";
        line += station.CanCraft(r, inventory) ? " [OK]" : " [manque]";
        lines.push_back(line);
    }
    DrawCenteredPanel("Atelier de transformation", lines);
}

void UI::DrawUpgradeMenu(const UpgradeStation& station, const UpgradeData& upgrades, const Inventory& inventory, int selected) {
    std::vector<std::string> lines;
    lines.push_back("Or : " + std::to_string(inventory.gold));
    lines.push_back("Entree : ameliorer | Echap : quitter");
    lines.push_back("");
    const auto& defs = station.Upgrades();
    for (size_t i = 0; i < defs.size(); ++i) {
        const auto& d = defs[i];
        int lvl = upgrades.Level(d.id);
        std::string matName = ItemNameOrId(d.materialId);
        std::string line = (static_cast<int>(i) == selected ? "> " : "  ") + d.name + " niv " + std::to_string(lvl) + "/" + std::to_string(d.maxLevel);
        line += " | " + std::to_string(station.GoldCost(d, upgrades)) + " or";
        if (!d.materialId.empty()) line += " + " + std::to_string(station.MaterialCost(d, upgrades)) + " " + matName;
        line += station.CanBuy(d, upgrades, inventory) ? " [OK]" : " [manque]";
        lines.push_back(line);
        if (static_cast<int>(i) == selected) lines.push_back("    " + d.description);
    }
    DrawCenteredPanel("Autel d'amelioration", lines);
}

void UI::DrawInventoryMenu(const Inventory& inventory, int selected) {
    const UiMetrics m = Metrics();
    const int panelW = std::min(1180, m.screenW - m.margin * 2);
    const int panelH = std::min(760, m.screenH - m.margin * 2 - 20);
    const int panelX = m.screenW / 2 - panelW / 2;
    const int panelY = m.screenH / 2 - panelH / 2;
    DrawPanel(Rectangle{static_cast<float>(panelX), static_cast<float>(panelY), static_cast<float>(panelW), static_cast<float>(panelH)});
    DrawHeader("Inventaire", Rectangle{static_cast<float>(panelX + 8), static_cast<float>(panelY + 8), static_cast<float>(panelW - 16), 46.0f});

    const int detailsW = std::min(330, std::max(250, panelW / 3));
    const int gridX = panelX + 24;
    const int gridY = panelY + 118;
    const int gridW = panelW - detailsW - 62;
    const int detailsX = panelX + panelW - detailsW - 24;
    const int detailsY = panelY + 74;
    const int detailsH = panelH - 98;
    const int cols = InventoryGridColumns();
    const int gap = 10;
    const int slot = std::max(54, std::min(78, (gridW - gap * (cols - 1)) / cols));

    DrawText(("Or " + std::to_string(inventory.gold) + "   Fragments " + std::to_string(inventory.ancientFragments)).c_str(), panelX + 24, panelY + 70, 18, GoldLine());
    DrawTextFit("Entree: equiper arme | A/R utilisables en jeu | Tab/Echap: fermer", panelX + 24, panelY + 94, 16, LIGHTGRAY, gridW);

    const char* cats[] = {"weapon", "consumable", "material", "loot", "relic", "other"};
    for (int i = 0; i < 6; ++i) {
        const char* cat = cats[i];
        const int count = CountCategory(inventory, cat);
        const std::string txt = CategoryLabel(cat) + " " + std::to_string(count);
        const int chipW = (detailsW - 10) / 2;
        const int chipX = detailsX + (i % 2) * (chipW + 10);
        const int chipY = panelY + 72 + (i / 2) * 28;
        DrawRectangle(chipX, chipY, chipW, 24, Color{31, 28, 36, 230});
        DrawRectangleLines(chipX, chipY, chipW, 24, GoldSoft());
        DrawTextFit(txt, chipX + 8, chipY + 5, 15, LIGHTGRAY, chipW - 16);
    }

    auto items = InventoryDisplayItems(inventory);
    if (items.empty()) {
        DrawText("Inventaire vide", gridX, gridY, 22, LIGHTGRAY);
    }

    const int gridBottom = panelY + panelH - 30;
    const int visibleRows = std::max(1, (gridBottom - gridY + gap) / (slot + gap));
    const int pageSize = std::max(1, visibleRows * cols);
    selected = std::max(0, std::min(selected, static_cast<int>(items.size()) - 1));
    const int page = items.empty() ? 0 : selected / pageSize;
    const int pageStart = page * pageSize;
    const int pageEnd = std::min(static_cast<int>(items.size()), pageStart + pageSize);

    for (int i = pageStart; i < pageEnd; ++i) {
        const int local = i - pageStart;
        const int col = local % cols;
        const int row = local / cols;
        const int x = gridX + col * (slot + gap);
        const int y = gridY + row * (slot + gap);

        const ItemDef* def = GetItemDef(items[i].first);
        const bool isSelected = i == selected;
        const Rectangle slotRect{static_cast<float>(x), static_cast<float>(y), static_cast<float>(slot), static_cast<float>(slot)};
        const bool hovered = CheckCollisionPointRec(GetMousePosition(), slotRect);
        const bool equipped = inventory.equippedWeapon == items[i].first;
        Color border = isSelected ? Color{255, 226, 128, 255} : (def && def->rare ? GoldLine() : GoldSoft());
        if (hovered && !isSelected) border = Color{228, 210, 156, 255};
        DrawRectangle(x, y, slot, slot, isSelected ? Color{48, 40, 41, 245} : (hovered ? Color{38, 34, 42, 245} : StoneSoft()));
        DrawRectangleLinesEx(slotRect, isSelected ? 3.0f : (hovered ? 2.2f : 1.4f), border);
        AddMenuHitbox(i, slotRect);
        DrawItemIcon(def, x + slot / 2, y + slot / 2 - 2, slot, equipped);
        if (items[i].second > 1) {
            std::string qty = "x" + std::to_string(items[i].second);
            DrawRectangle(x + slot - 34, y + slot - 21, 31, 18, Ink());
            DrawText(qty.c_str(), x + slot - 31, y + slot - 18, 14, WHITE);
        }
    }
    if (!items.empty() && static_cast<int>(items.size()) > pageSize) {
        const int pageCount = (static_cast<int>(items.size()) + pageSize - 1) / pageSize;
        const std::string pageText = "Page " + std::to_string(page + 1) + "/" + std::to_string(pageCount);
        DrawText(pageText.c_str(), gridX, gridBottom + 2, 16, LIGHTGRAY);
    }

    const int detailPanelY = detailsY + 118;
    const int detailPanelH = std::max(92, detailsH - 118);
    const int detailBottom = detailPanelY + detailPanelH - 16;
    DrawPanel(Rectangle{static_cast<float>(detailsX), static_cast<float>(detailPanelY), static_cast<float>(detailsW), static_cast<float>(detailPanelH)}, Color{15, 14, 19, 232}, GoldSoft());
    int detailLineY = detailPanelY + 18;
    auto drawDetail = [&](const std::string& text, int fontSize, Color color) {
        if (detailLineY + fontSize <= detailBottom) {
            DrawTextFit(text, detailsX + 18, detailLineY, fontSize, color, detailsW - 36);
        }
        detailLineY += fontSize + 9;
    };

    if (!items.empty()) {
        const auto& current = items[selected];
        const ItemDef* def = GetItemDef(current.first);
        const std::string name = def ? def->name : current.first;
        Color accent = CategoryColor(def);
        drawDetail(name, 23, accent);
        drawDetail("Categorie : " + CategoryLabel(def ? def->category : "other"), 17, WHITE);
        drawDetail("Quantite : " + std::to_string(current.second), 17, LIGHTGRAY);
        drawDetail("Valeur : " + std::to_string(def ? def->sellPrice : 0) + " or/u", 17, GoldLine());
        drawDetail(def && def->rare ? "Rarete : rare" : "Rarete : commune", 17, def && def->rare ? GoldLine() : LIGHTGRAY);
        if (def && def->category == "weapon") {
            drawDetail("Degats +" + std::to_string(static_cast<int>(def->damageBonus)), 17, WHITE);
            drawDetail("Portee +" + std::to_string(static_cast<int>(def->rangeBonus * 10.0f) / 10.0f), 17, WHITE);
            drawDetail("Vitesse " + std::to_string(static_cast<int>(def->attackSpeedBonus * 100.0f)) + "%", 17, WHITE);
            drawDetail(inventory.equippedWeapon == current.first ? "Action : deja equipee" : "Action : Entree pour equiper", 17, Turquoise());
        } else {
            drawDetail("Action : aucune action directe ici", 17, LIGHTGRAY);
        }
    } else {
        drawDetail("Aucun objet", 22, LIGHTGRAY);
    }
}

void UI::DrawChestMenu(const Inventory& inventory, int selected, bool chestSide) {
    std::vector<std::string> lines;
    lines.push_back(chestSide ? "Coffre selectionne - Entree: retirer 1 | C: changer cote" : "Inventaire selectionne - Entree: deposer 1 | C: changer cote");
    lines.push_back("Echap : fermer");
    lines.push_back("");
    auto list = chestSide ? inventory.SortedChest() : inventory.SortedItems();
    if (list.empty()) lines.push_back(chestSide ? "Coffre vide" : "Inventaire vide");
    for (size_t i = 0; i < list.size(); ++i) {
        lines.push_back((static_cast<int>(i) == selected ? "> " : "  ") + ItemNameOrId(list[i].first) + " x" + std::to_string(list[i].second));
    }
    DrawCenteredPanel("Coffre personnel", lines);
    const UiMetrics m = Metrics();
    const int w = std::min(980, m.screenW - m.margin * 2);
    const int h = std::min(std::min(760, m.screenH - m.margin * 2), 120 + static_cast<int>(lines.size()) * 28);
    const int x = m.screenW / 2 - w / 2;
    const int y = m.screenH / 2 - h / 2;
    const Rectangle button{static_cast<float>(x + w - 214), static_cast<float>(y + 96), 174.0f, 26.0f};
    AddMenuHitbox(0, button, ACTION_CHEST_TOGGLE_SIDE);
    DrawRectangleRec(button, FadeColor(GoldSoft(), 0.45f));
    DrawRectangleLinesEx(button, 1.0f, GoldLine());
    DrawTextFit("Changer cote", static_cast<int>(button.x) + 12, static_cast<int>(button.y) + 5, 15, WHITE, 150);
}

void UI::DrawCenteredPanel(const std::string& title, const std::vector<std::string>& lines) {
    const UiMetrics m = Metrics();
    const int w = std::min(980, m.screenW - m.margin * 2);
    const int h = std::min(std::min(760, m.screenH - m.margin * 2), 120 + static_cast<int>(lines.size()) * 28);
    const int x = m.screenW / 2 - w / 2;
    const int y = m.screenH / 2 - h / 2;
    DrawPanel(Rectangle{static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h)});
    DrawTextFit(title, x + 24, y + 18, 30, GoldLine(), w - 48);
    int yy = y + 68;
    int interactiveIndex = 0;
    for (const auto& line : lines) {
        const bool interactive = line.size() > 2 && (line.rfind("> ", 0) == 0 || (line.rfind("  ", 0) == 0 && line[2] != ' '));
        Rectangle hit{static_cast<float>(x + 24), static_cast<float>(yy - 3), static_cast<float>(w - 48), 26.0f};
        if (interactive) {
            const bool hovered = CheckCollisionPointRec(GetMousePosition(), hit);
            if (hovered) DrawRectangleRec(hit, FadeColor(GoldSoft(), 0.28f));
            AddMenuHitbox(interactiveIndex, hit);
        }
        DrawTextFit(line, x + 32, yy, 19, WHITE, w - 64);
        yy += 28;
        if (interactive) ++interactiveIndex;
        if (yy > y + h - 32) break;
    }
}

int UI::InventoryGridColumns() {
    const int w = std::max(640, GetScreenWidth());
    if (w < 760) return 4;
    if (w < 980) return 5;
    if (w < 1200) return 6;
    if (w < 1500) return 7;
    return 8;
}

std::vector<std::pair<std::string, int>> UI::InventoryDisplayItems(const Inventory& inventory) {
    std::vector<std::pair<std::string, int>> out(inventory.items.begin(), inventory.items.end());
    std::sort(out.begin(), out.end(), [](const auto& a, const auto& b) {
        const ItemDef* da = GetItemDef(a.first);
        const ItemDef* db = GetItemDef(b.first);
        const int ra = CategoryRank(da);
        const int rb = CategoryRank(db);
        if (ra != rb) return ra < rb;
        const std::string na = da ? da->name : a.first;
        const std::string nb = db ? db->name : b.first;
        return na < nb;
    });
    return out;
}
