#pragma once

#include "basetool.h"
#include <vector>

class CToolBox
{
public:
	CToolBox();

	void ShowToolBox();
	void Update();

	void SetTool(CBaseTool* tool);
	void SwitchToLast();

	std::vector<CBaseTool*> m_tools;
	CBaseTool* m_currentTool;
	CBaseTool* m_lastTool;

	// This is for quick hold key binded tools
	bool m_holdingTool;
	CBaseTool* m_pocketedTool;
};