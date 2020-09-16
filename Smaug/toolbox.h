#pragma once

#include "basetool.h"
#include <vector>

class CToolBox
{
public:

	void ShowToolBox();
	void Update();

	std::vector<CBaseTool*> m_tools;
	CBaseTool* m_currentTool;

};