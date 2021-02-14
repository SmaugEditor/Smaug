#include "toolbox.h"
#include "smaugapp.h"
#include "cursor.h"

#include <imgui.h>

#define EMPTY_TOOL_MODEL "assets/gizmo.obj"

CToolBox::CToolBox()
{
	m_currentTool = nullptr;
	m_lastTool = nullptr;
	m_pocketedTool = nullptr;
	m_holdingTool = false;
}

void CToolBox::RegisterTool(CBaseTool* tool)
{
	tool->Init();
	
	m_tools.push_back(tool);
}

void CToolBox::ShowToolBox()
{
	// UI
	if (ImGui::Begin("Tool Box", nullptr, ImGuiWindowFlags_NoScrollbar))
	{
		ImVec2 contentRegion = ImGui::GetContentRegionAvail();
		
		// We want to take up as much space as we can with the buttons
		ImVec2 buttonSize = ImVec2(fmin(contentRegion.x, contentRegion.y), 0);
		buttonSize.y = buttonSize.x;


		// If we're wider than we are tall, we want to same line all of our buttons
		bool sameLine = contentRegion.y < contentRegion.x;


		for (int i = 0; i < m_tools.size(); i++)
		{
			CBaseTool* tool = m_tools[i];
			
			ImVec4 tint(1,1,1,1);

			// If this tool is selected, it gets to have a fancy new color!
			if (tool == m_currentTool)
			{
				tint.y = 0.75f;
				tint.z = 0.75f;
			}

			// Add the button
			if (ImGui::ImageButton((ImTextureID)tool->GetIconTexture().idx, buttonSize, ImVec2(0,0), ImVec2(1,1), 0, ImVec4(0,0,0,0), tint))
			{
				// If it's our current tool, deselect it
				if (m_currentTool == tool)
					SetTool(nullptr);
				else
					SetTool(tool);
			}

			if (ImGui::IsItemHovered())
			{
				// We want to say the tool's name when we hover
				ImGui::BeginTooltip();
				ImGui::SetTooltip(tool->GetName());
				ImGui::EndTooltip();
			}


			if (sameLine)
				ImGui::SameLine();

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
		if (tool->GetHoldKey() != GLFW_KEY_UNKNOWN)
		{
			if (m_holdingTool && m_currentTool == tool)
			{
				// If we're holding this tool and not holding its key, deselect it
				if (glfwGetKey(GetApp().GetWindow(), tool->GetHoldKey()) == GLFW_RELEASE)
				{
					m_currentTool->Disable();

					m_currentTool = m_pocketedTool;
					if (m_currentTool)
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
					if (m_currentTool)
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
		if (tool->GetToggleKey() != GLFW_KEY_UNKNOWN)
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
}

void CToolBox::Update(float dt, glm::vec3 mousePos)
{

	// If we have a current tool selected, update it
	if (m_currentTool)
		m_currentTool->Update(dt, mousePos);
	else
		GetCursor().SetPosition(mousePos);
}

void CToolBox::SetTool(CBaseTool* tool)
{

	m_lastTool = m_currentTool;
	if(m_lastTool)
		m_lastTool->Disable();
	m_currentTool = tool;
	if (m_currentTool)
		m_currentTool->Enable();
	else
		GetCursor().SetModel(EMPTY_TOOL_MODEL);
}

void CToolBox::SwitchToLast()
{
	SetTool(m_lastTool);
}
