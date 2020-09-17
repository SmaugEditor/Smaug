#include "toolbox.h"
#include "imgui.h"

void CToolBox::ShowToolBox()
{
	
}

void CToolBox::Update()
{
	// UI
	if (ImGui::Begin("Tool Box"))
	{
		bool colorChange = false;
		for (int i = 0; i < m_tools.size(); i++)
		{
			// If this tool is selected, it gets to have a fancy new color!
			if (m_tools[i] == m_currentTool)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(255, 0, 0, 255));
				colorChange = true;
			}
			if (ImGui::Button(m_tools[i]->GetName()))
			{
				if (m_currentTool)
					m_currentTool->Disable();

				if (m_currentTool == m_tools[i])
					m_currentTool = nullptr;
				else
				{
					m_currentTool = m_tools[i];
					m_currentTool->Enable();
				}
			}
			if (colorChange)
			{
				ImGui::PopStyleColor();
				colorChange = false;
			}

		}
		ImGui::End();
	}

	if (m_currentTool)
		m_currentTool->Update();
}
