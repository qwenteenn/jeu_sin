#pragma once
#include "Common.h"
#include "Boss.h"
#include "CraftingStation.h"
#include "Inventory.h"
#include "Player.h"
#include "Room.h"
#include "Shop.h"
#include "UpgradeStation.h"

class UI {
public:
    static constexpr int ACTION_PRIMARY = 0;
    static constexpr int ACTION_VENDOR_SELL_ALL = 1;
    static constexpr int ACTION_CHEST_TOGGLE_SIDE = 2;

    static void BeginFrame();
    static void AddMenuHitbox(int index, Rectangle rect, int action = ACTION_PRIMARY);
    static int HoveredMenuIndex();
    static int HoveredMenuAction();
    static bool WasMenuItemClicked();
    static void DrawBar(int x, int y, int w, int h, float pct, Color fill, const std::string& label);
    static void DrawHud(const Player& player, const Inventory& inventory, int floor, const Room* room, const Boss& boss, const std::string& message);
    static void DrawFirstPersonOverlay(const Player& player, const Inventory& inventory);
    static void DrawHelp();
    static void DrawClassSelection(int selected);
    static void DrawShopMenu(const std::string& title, const Shop& shop, const Inventory& inventory, int selected);
    static void DrawVendorMenu(const Vendor& vendor, const Inventory& inventory, int selected);
    static void DrawCraftingMenu(const CraftingStation& station, const Inventory& inventory, int selected);
    static void DrawUpgradeMenu(const UpgradeStation& station, const UpgradeData& upgrades, const Inventory& inventory, int selected);
    static void DrawInventoryMenu(const Inventory& inventory, int selected);
    static void DrawChestMenu(const Inventory& inventory, int selected, bool chestSide);
    static void DrawCenteredPanel(const std::string& title, const std::vector<std::string>& lines);
    static int InventoryGridColumns();
    static std::vector<std::pair<std::string, int>> InventoryDisplayItems(const Inventory& inventory);
};
