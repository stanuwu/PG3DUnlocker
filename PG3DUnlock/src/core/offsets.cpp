#include "offsets.h"

#include <Psapi.h>

namespace offsets
{
    struct scan
    {
        pattern* pattern;
        bool success;
    };

    scan makeScanObj(pattern* pattern)
    {
        return {pattern, false};
    }

    bool init()
    {
        size_t found = 0;
        scan scans[] = {
            makeScanObj(&emit),
            makeScanObj(&weapon_sounds_update),
            makeScanObj(&get_progress_updater_instance),
            makeScanObj(&progress_updater_add_currency),
            makeScanObj(&lottery_get_price),
            makeScanObj(&lottery_get_count),
            makeScanObj(&get_data_holder_instance),
            makeScanObj(&data_holder_get_data_list),
            makeScanObj(&add_item),
            makeScanObj(&add_weapon),
            makeScanObj(&add_pets),
            makeScanObj(&add_wear),
            makeScanObj(&create_string_from_encoding),
            makeScanObj(&get_utf8_encoding),
        };
        size_t patternCount = sizeof(scans) / sizeof(scan);

        auto module = "GameAssembly.dll";
        HMODULE clientBase = GetModuleHandleA(module);
        MODULEINFO clientInfo;
        GetModuleInformation(GetCurrentProcess(), clientBase, &clientInfo, sizeof(clientInfo));

        // each byte
        for (DWORD i = 0; i < clientInfo.SizeOfImage; ++i)
        {
            // each pattern
            for (size_t j = 0; j < patternCount; ++j)
            {
                if (scans[j].success) continue;
                bool res = scans[j].pattern->Scan(static_cast<byte*>(clientInfo.lpBaseOfDll) + i);
                if (res)
                {
                    scans[j].success = true;
                    found++;
                }
            }
        }

        return found >= patternCount;
    }
}
