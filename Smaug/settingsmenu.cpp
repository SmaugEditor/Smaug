#include "settingsmenu.h"
#include "imgui.h"
#include "svarex.h"

CSettingsRegister& GetSettingsRegister()
{
    static CSettingsRegister s_settingsRegister;
    return s_settingsRegister;
}

void CSettingsMenu::DrawMenu()
{
    if (ImGui::Begin("Settings"))
    {
        std::vector<CSettingsLink*>& linkList = GetSettingsRegister().m_linkList;
        
        // Display a selection list for all settings
        for (size_t i = 0; i < linkList.size(); i++)
        {
            if (ImGui::BeginTabBar("SettingsTab"))
            {
                if (ImGui::TabItemButton(linkList[i]->m_displayName))
                    m_selectedTabIndex = i;
                ImGui::EndTabBar();
            }
        }

        // Display the selected tab
        if (m_selectedTabIndex < linkList.size())
        {
            CSettingsLink* link = linkList[m_selectedTabIndex];
            ImGui::Text(link->m_displayName);
            for (size_t i = 0; i < link->m_table->m_varTable.size(); i++)
            {
                ISVar* var = link->m_table->m_varTable[i];
                
                //Figure out what type of edit prompt to display...
                const type_info& info = link->m_table->m_varTable[i]->GetTypeInfo();
                if (info == typeid(int))
                {
                    ImGui::InputInt(var->GetName(), (int*)var->GetData(), 0);
                }
                else if (info == typeid(bool))
                {
                    ImGui::Checkbox(var->GetName(), (bool*)var->GetData());
                }
                else if (info == typeid(float))
                {
                    ImGui::InputFloat(var->GetName(), (float*)var->GetData());
                }
                else if (info == typeid(float))
                {
                    ImGui::InputFloat(var->GetName(), (float*)var->GetData());
                }
                else if (info == typeid(glm::vec3))
                {
                    ImGui::InputFloat3(var->GetName(), (float*)var->GetData());
                }
                else
                {
                    // Failed to figure it out. Default to a string edit.
                    ImGui::Text(var->GetName());
                }

            }

            if (ImGui::Button("Apply Settings"))
            {
                // Save File
            }
        }
        ImGui::End();
    }
}
