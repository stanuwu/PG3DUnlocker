#include "menu.h"

#include <iostream>
#include <windows.h>

#include "../features/ws.h"
#include "../features/wsSerial.h"
#include "imgui_stdlib.h"
#include "../features/unlock.h"

namespace menu
{
    bool is_active()
    {
        return draw_imgui == true;
    }

    void draw()
    {
        theme();
        ImGui::GetMouseCursor();
        ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
        ImGui::GetIO().WantCaptureMouse = is_active();
        ImGui::GetIO().MouseDrawCursor = is_active();
        if (is_active())
        {
            ImGui::SetNextWindowSize(ImVec2(350, 630), ImGuiCond_Once);
            ImGui::SetNextWindowBgAlpha(1.0f);
            if (ImGui::Begin("PG3DUnlocker by stan", &draw_imgui,
                             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoScrollbar))
            {
                ImGui::Text("Warning!");
                ImGui::Text("Adding too much can lead to a ban!");
                static bool safeMode = true;

                ImGui::SeparatorText("Currency");
                static int currencySelected = 0;
                static int currencyAmount = 1;
                ImGui::Combo("Currency", &currencySelected, currency_names, sizeof(currency_names) / sizeof(char*));
                ImGui::SliderInt("Amount", &currencyAmount, 1, safeMode ? 2000 : 1000000);
                if (ImGui::Button("Add##Currency"))
                {
                    unlock::queueCurrency({currency_names[currencySelected], currencyAmount});
                }

                ImGui::SeparatorText("Lottery");
                ImGui::Checkbox("Change Price", &unlock::changePrice);
                ImGui::SliderInt("Price", &unlock::newPrice, safeMode ? -10000 : -1000, 1);
                ImGui::Checkbox("Change Count", &unlock::changeCount);
                ImGui::SliderInt("Count", &unlock::newCount, 1, safeMode ? 1000 : 100000);

                ImGui::SeparatorText("Item");
                ImGui::Combo("Type", &unlock::selectedType, unlock::itemTypeArray, IM_ARRAYSIZE(unlock::itemTypeArray));
                std::vector<std::string> currentList = unlock::items[unlock::itemTypes[unlock::itemTypeArray[
                    unlock::selectedType]]];
                if (currentList.size() <= unlock::selectedItem) unlock::selectedItem = 0;
                if (ImGui::BeginCombo(
                    "Item",
                    currentList[unlock::selectedItem].
                    c_str()))
                {
                    static std::string search("");
                    ImGui::InputText("Search", &search);
                    for (size_t i = 0; i < currentList.size(); ++i)
                    {
                        if (search.empty() || currentList[i].find(search) != std::string::npos)
                        {
                            if (ImGui::Selectable(currentList[i].c_str()))
                            {
                                unlock::selectedItem = static_cast<int>(i);
                            }
                        }
                    }
                    ImGui::EndCombo();
                }
                if (ImGui::Button("Add##Item"))
                {
                    unlock::queueItem({
                        unlock::itemUnlockType[unlock::itemTypeArray[unlock::selectedType]],
                        currentList[unlock::selectedItem]
                    });
                }

                ImGui::SeparatorText("Lottery");
                ImGui::Checkbox("Change Price", &unlock::rewardMultiplier);
                ImGui::SliderInt("Price", &unlock::rewardMultiplierAmount, 1, safeMode ? 5 : 100);

                ImGui::SeparatorText("Danger Zone");
                ImGui::Checkbox("Disable Limits", &safeMode);

                ImGui::SeparatorText("Experimental");
                ImGui::Checkbox("Log Websocket Emit", &ws::logWebsocket);

                ImGui::SeparatorText("");
                ImGui::Text("Press Insert to toggle menu");
                ImGui::End();
            }
        }

        if (GetAsyncKeyState(VK_INSERT) & 1)
        {
            draw_imgui = !draw_imgui;
        }
    }
}
