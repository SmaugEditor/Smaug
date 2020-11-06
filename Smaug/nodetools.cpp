#include "nodetools.h"
#include "svarex.h"
#include "settingsmenu.h"


BEGIN_SVAR_TABLE(CNodeToolsSettings)
	DEFINE_TABLE_SVAR_KEY(dragToolToggle,    GLFW_KEY_G)
	DEFINE_TABLE_SVAR_KEY(dragToolHold,		 GLFW_KEY_UNKNOWN)
	DEFINE_TABLE_SVAR_KEY(extrudeToolToggle, GLFW_KEY_UNKNOWN)
	DEFINE_TABLE_SVAR_KEY(extrudeToolHold,   GLFW_KEY_LEFT_SHIFT)
END_SVAR_TABLE()

static CNodeToolsSettings s_NodeToolsSettings;
DEFINE_SETTINGS_MENU("Node Tools", s_NodeToolsSettings);

int CDragTool::GetToggleKey() { return s_NodeToolsSettings.dragToolToggle.GetValue().key; }
int CDragTool::GetHoldKey() { return s_NodeToolsSettings.dragToolHold.GetValue().key; }


int CExtrudeTool::GetToggleKey() { return s_NodeToolsSettings.extrudeToolToggle.GetValue().key; }
int CExtrudeTool::GetHoldKey() { return s_NodeToolsSettings.extrudeToolHold.GetValue().key; }
