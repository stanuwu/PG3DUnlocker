#include "unlock.h"
#include <windows.h>
#include "detours/detours.h"

#include "../core/offsets.h"
#include "../sdk/il2cppApi.h"
#include "../sdk/il2cppDict.h"
#include "../util/util.h"

namespace unlock
{
    void dumpAllItems()
    {
        auto fnGetDataHolder = static_cast<void*(*)()>((void*)offsets::get_data_holder_instance.GetAddress());
        auto fnGetDataList = static_cast<il2cpp::list<il2cpp::string*>*(*)(void*, int)>((void*)
            offsets::data_holder_get_data_list.GetAddress());

        items.emplace(-1, std::vector<std::string>());
        for (auto entry : itemTypes)
        {
            if (entry.second == -1) continue;
            auto name = entry.first;
            auto dict = fnGetDataList(
                fnGetDataHolder(), entry.second);
            auto list = std::vector<std::string>();

            for (int i = 0; i < dict->m_pListArray->m_uMaxLength; ++i)
            {
                if (dict->m_pListArray->At(i) == nullptr) break;
                std::string key = util::clean(dict->m_pListArray->At(i)->ToString());
                if (key == "") continue;
                list.push_back(key);
                if (entry.second == 1070 && key.starts_with("avatar"))
                {
                    items[-1].push_back(key);
                }
            }

            items.emplace(entry.second, list);
        }
    }

    bool init()
    {
        dumpAllItems();

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        oWeaponSoundsUpdate = reinterpret_cast<void(*)(void*)>(offsets::weapon_sounds_update.GetAddress());
        oLotteryGetPrice = reinterpret_cast<int(*)(void*)>(offsets::lottery_get_price.GetAddress());
        oLotteryGetCount = reinterpret_cast<int(*)(void*)>(offsets::lottery_get_count.GetAddress());

        DetourAttach(&(PVOID&)(oWeaponSoundsUpdate), hkWeaponSoundsUpdate);
        DetourAttach(&(PVOID&)(oLotteryGetPrice), hkLotteryGetPrice);
        DetourAttach(&(PVOID&)(oLotteryGetCount), hkLotteryGetCount);

        LONG status = DetourTransactionCommit();
        if (status != NO_ERROR)
        {
            return false;
        }

        return true;
    }

    void queueCurrency(currencyQueueItem item)
    {
        std::lock_guard lock(currencyMutex);
        currencyQueue.push_back(item);
    }

    void queueItem(itemQueueItem item)
    {
        std::lock_guard lock(itemMutex);
        itemQueue.push_back(item);
    }

    void addCurrency(std::string type, int amount)
    {
        struct itemObtainParams
        {
            int field1;
            int field2;
            int field3;
            int field4;
            int field5;
            int field6;
            int field7;
            bool field8;
            int field9;
            int field10;
            bool field11;
            int field12;
            int field13;
            bool field14;
            int field15;
            int field16;
            int field17;
            bool field18;
            int64_t field19;
            int field20;
            bool field21;
            bool field22;
            bool field23;
            bool field24;
            int field25;
        };

        static const auto fnGetProgressUpdater = static_cast<void*(*)()>((void*)
            offsets::get_progress_updater_instance.
            GetAddress());
        void* progress_instance = fnGetProgressUpdater();
        itemObtainParams params = {
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            false,
            0,
            0,
            false,
            0,
            0,
            false,
            0,
            0,
            0,
            false,
            0,
            0,
            false,
            false,
            false,
            false,
            0
        };
        static const auto fnAddCurrency = static_cast<void(*)(void*, void*, int, int, bool, bool, void*)>((void*)
            offsets::progress_updater_add_currency.GetAddress());
        // 8 = inapp bonus
        fnAddCurrency(progress_instance, il2cpp::CreateManagedString(type), amount, 8, false, false, &params);
    }

    void addItem(int type, std::string key)
    {
        auto array = new short[12];
        void* magic_object = nullptr;
        if (type == 8)
        {
            auto fnAddWear = reinterpret_cast<void (*)(il2cpp::string*)>(
                offsets::add_wear.GetAddress());
            fnAddWear(
                il2cpp::CreateManagedString(key));
            return;
        }
        if (type == 2)
        {
            auto fnAddWeapon = reinterpret_cast<void (*)(il2cpp::string*, bool, bool)>(
                offsets::add_weapon.GetAddress());
            fnAddWeapon(
                il2cpp::CreateManagedString(key), true, false);
            return;
        }
        if (type == -1)
        {
            auto fnAddPets = reinterpret_cast<void (*)(il2cpp::string*, int)>(
                offsets::add_pets.GetAddress());
            fnAddPets(
                il2cpp::CreateManagedString(key), 65);
            return;
        }
        auto fnAddItem = reinterpret_cast<void (*)(int, il2cpp::string*, void*, void*, bool, bool, void*, void*, int,
                                                   void*)>(
            offsets::add_item.GetAddress());
        fnAddItem(
            type,
            il2cpp::CreateManagedString(key),
            magic_object, nullptr, true, false, nullptr, nullptr, 21, array);
        delete[] array;
    }

    void __fastcall hkWeaponSoundsUpdate(void* weaponSounds)
    {
        std::lock_guard lock(currencyMutex);
        while (currencyQueue.size() > 0)
        {
            auto item = currencyQueue.back();
            currencyQueue.pop_back();
            addCurrency(item.type, item.amount);
        }

        std::lock_guard lock2(itemMutex);
        while (itemQueue.size() > 0)
        {
            auto item = itemQueue.back();
            itemQueue.pop_back();
            addItem(item.type, item.key);
        }

        oWeaponSoundsUpdate(weaponSounds);
    }

    int __fastcall hkLotteryGetPrice(void* instance)
    {
        if (changePrice) return newPrice;
        return oLotteryGetPrice(instance);
    }

    int __fastcall hkLotteryGetCount(void* instance)
    {
        if (changeCount) return newCount;
        return oLotteryGetPrice(instance);
    }
}
