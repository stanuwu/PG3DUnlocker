#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <map>

#include "../sdk/il2cppString.h"

namespace unlock
{
    struct currencyQueueItem
    {
        std::string type;
        int amount;
    };

    struct itemQueueItem
    {
        int type;
        std::string key;
    };


    inline auto currencyMutex = std::mutex();
    inline auto currencyQueue = std::vector<currencyQueueItem>();

    inline bool changePrice = false;
    inline int newPrice = 1;
    inline bool changeCount = false;
    inline int newCount = 1;

    inline bool rewardMultiplier = false;
    inline int rewardMultiplierAmount = 1;

    inline auto itemMutex = std::mutex();
    inline auto itemQueue = std::vector<itemQueueItem>();
    inline int selectedType = 0;
    inline int selectedItem = 0;

    inline std::map<int, std::vector<std::string>> items = {};

    inline const char* itemTypeArray[] = {
        "Weapons",
        "Armor",
        "Mask",
        "Hat",
        "Boots",
        "Cape",
        "Pet",
        "Avatar",
        "Royale"
    };

    inline std::map<std::string, int> itemUnlockType = {
        {"Weapons", 2},
        {"Armor", 7},
        {"Mask", 12},
        {"Hat", 6},
        {"Boots", 10},
        {"Cape", 9},
        {"Avatar", 8},
        {"Pet", -1},
        {"Royale", 8},
    };

    inline std::map<std::string, int> itemTypes = {
        {"Avatar", -1},
        {"None", 0},
        {"Weapons", 10},
        {"Armor", 20},
        {"Mask", 30},
        {"Hat", 40},
        {"Boots", 50},
        {"Cape", 60},
        {"Skin", 65},
        {"Gadget", 70},
        {"Pet", 80},
        {"Egg", 83},
        {"LobbyItem", 85},
        {"FortItem", 90},
        {"Gems", 1000},
        {"Coins", 1010},
        {"Leprechaun", 1020},
        {"WeaponUpgrade", 1030},
        {"GachaFreeSpin", 1040},
        {"EventCurrency", 1050},
        {"VIP", 1060},
        {"Parts", 1070},
        {"Royale", 1080},
        {"BattlePassLevel", 1090},
        {"BattlePassExp", 1100},
        {"BattlePassCurrency", 1110},
        {"GoldenSkin", 1120},
        {"EventChest", 1130},
        {"CraftCurrency", 1140},
        {"Module", 1150},
        {"ModulePoint", 1155},
        {"ModuleChest", 1160},
        {"WeaponSkin", 1170},
        {"ClanCurrency", 1180},
        {"Coupons", 1190},
        {"Currency", 1200},
        {"Character", 1210},
        {"ClanShields", 1220},
        {"ClanLootBox", 1230},
        {"ClanPlaceable", 1240},
        {"ClanPlaceablePoint", 1250},
        {"Detail", 1300},
        {"WeaponLevelUpgrade", 1310},
        {"PlayerBuff", 1320},
        {"ClanBuff", 1330},
        {"WeaponQualityUpgrade", 1340},
        {"ArmorSkin", 1350},
        {"ClanBuilding", 1360},
        {"ClanBuildingPoint", 1370},
        {"FreeUpgrade", 1380},
        {"Chest", 1390},
        {"Exp", 1400},
        {"Stats", 1410},
        {"ModeSlots", 1420},
        {"Executable", 1430},
        {"Tank", 1440},
        {"VIP20", 1450},
        {"LootBox", 1460},
        {"Graffiti", 1470},
        {"PixelPassExp", 1490},
        {"ClanRankExperience", 1500},
        {"WearSkin", 1510},
        {"Applicable", 1520},
        {"CraftSet", 1530},
        {"FeatureExp", 1540},
        {"PackagedItem", 1550},
        {"Achievement", 1560},
        {"ExpirySimple", 1570},
        {"Static", 1580},
        {"GemsHarvester", 1590},
    };

    bool init();
    void queueCurrency(currencyQueueItem item);
    void queueItem(itemQueueItem item);
    inline void (*oWeaponSoundsUpdate)(void* weaponSounds);
    void __fastcall hkWeaponSoundsUpdate(void* weaponSounds);
    inline int (*oLotteryGetPrice)(void* instance);
    int __fastcall hkLotteryGetPrice(void* instance);
    inline int (*oLotteryGetCount)(void* instance);
    int __fastcall hkLotteryGetCount(void* instance);
    inline int (*oGetRewardMultiplier)(void* instance);
    int __fastcall hkGetRewardMultiplier(void* instance);
};
