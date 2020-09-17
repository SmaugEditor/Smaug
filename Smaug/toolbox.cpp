#include "toolbox.h"
#include "imgui.h"
#include "smaugapp.h"

CToolBox::CToolBox()
{
	m_currentTool = nullptr;
	m_lastTool = nullptr;
	m_pocketedTool = nullptr;
	m_holdingTool = false;
}

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
				if (m_currentTool == m_tools[i])
					SetTool(nullptr);
				else
					SetTool(m_tools[i]);
			}
			if (colorChange)
			{
				ImGui::PopStyleColor();
				colorChange = false;
			}

		}
		ImGui::End();
	}


	// I could use GetApp()->isKeyDown, but then I'm at the whims of ImGui stealing control...
	// Find a better solution for this!

	// Check for keybinds
	for (int i = 0; i < m_tools.size(); i++)
	{
		CBaseTool* tool = m_tools[i];

		// Does this tool have a hold key?
		if (tool->GetHoldKey())
		{
			if (m_holdingTool && m_currentTool == tool)
			{
				// If we're holding this tool and not holding its key, deselect it
				if (glfwGetKey(GetApp().GetWindow(), tool->GetHoldKey()) == GLFW_RELEASE)
				{
					m_currentTool->Disable();

					m_currentTool = m_pocketedTool;
					if(m_currentTool)
						m_currentTool->Enable();

					m_pocketedTool = nullptr;
					m_holdingTool = false;
	
					break;
				}
			}
			else
			{
				// Check if we want this tool
				if (glfwGetKey(GetApp().GetWindow(), tool->GetHoldKey()) == GLFW_PRESS && glfwGetKey)
				{
					// Put our current tool in our pocket
					if(m_currentTool)
						m_currentTool->Disable();
					m_pocketedTool = m_currentTool;

					// And swap in our requested
					m_currentTool = tool;
					m_currentTool->Enable();

					m_holdingTool = true;

					// We got our tool, now let's split!
					break;
				}
			}

		}

		// Does this tool have a toggle key?
		if (tool->GetToggleKey())
		{
			if (glfwGetKey(GetApp().GetWindow(), tool->GetToggleKey()) == GLFW_PRESS)
			{
				if (m_currentTool == tool)
				{
					// If we're already holding this tool, switch back to the last tool
					SwitchToLast();
				}
				else
				{
					SetTool(tool);
				}
				break;
			}
		}
	}

	// If we have a current tool selected, update it
	if (m_currentTool)
		m_currentTool->Update();
}

void CToolBox::SetTool(CBaseTool* tool)
{

	m_lastTool = m_currentTool;
	if(m_lastTool)
		m_lastTool->Disable();
	m_currentTool = tool;
	if (m_currentTool)
		m_currentTool->Enable();
}

void CToolBox::SwitchToLast()
{
	SetTool(m_lastTool);
}
