#pragma once

#include "basetool.h"
#include <vector>

class CToolBox
{
public:
	CToolBox();

	void ShowToolBox();
	void Update(float dt, glm::vec3 mousePos);

	void SetTool(CBaseTool* tool);
	void SwitchToLast();

	// This should always be used instead of directly adding a tool to m_tools. It insures the tool is properly initialized and etc
	void RegisterTool(CBaseTool* tool);

	std::vector<CBaseTool*> m_tools;
	CBaseTool* m_currentTool;
	CBaseTool* m_lastTool;

	// This is for quick hold key binded tools
	bool m_holdingTool;
	CBaseTool* m_pocketedTool;
};