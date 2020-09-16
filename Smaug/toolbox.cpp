#include "toolbox.h"
#include "imgui.h"

void CToolBox::ShowToolBox()
{
	
}

void CToolBox::Update()
{
	if (ImGui::Begin("Tool Box"))
	{
		for (int i = 0; i < m_tools.size(); i++)
		{
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
		}
		ImGui::End();
	}

	if (m_currentTool)
		m_currentTool->Update();
}
