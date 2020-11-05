#include "nodetools.h"
#include "svarex.h"
#include "settingsmenu.h"


BEGIN_SVAR_TABLE(CNodeToolsSettings)
	DEFINE_TABLE_SVAR(dragToolToggle,    GLFW_KEY_G)
	DEFINE_TABLE_SVAR(dragToolHold,      0)
	DEFINE_TABLE_SVAR(extrudeToolToggle, 0)
	DEFINE_TABLE_SVAR(extrudeToolHold,   GLFW_KEY_LEFT_SHIFT)
END_SVAR_TABLE()

static CNodeToolsSettings s_NodeToolsSettings;
DEFINE_SETTINGS_MENU("Node Tools", s_NodeToolsSettings);

int CDragTool::GetToggleKey() { return s_NodeToolsSettings.dragToolToggle.GetValue(); }
int CDragTool::GetHoldKey() { return s_NodeToolsSettings.dragToolHold.GetValue(); }


int CExtrudeTool::GetToggleKey() { return s_NodeToolsSettings.extrudeToolToggle.GetValue(); }
int CExtrudeTool::GetHoldKey() { return s_NodeToolsSettings.extrudeToolHold.GetValue(); }
