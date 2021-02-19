#include "nodetools.h"
#include "svarex.h"
#include "settingsmenu.h"


BEGIN_SVAR_TABLE(CNodeToolsSettings)
	DEFINE_TABLE_SVAR_INPUT(dragToolToggle,    GLFW_KEY_G,          false)
	DEFINE_TABLE_SVAR_INPUT(dragToolHold,	   GLFW_KEY_UNKNOWN,    false)
	DEFINE_TABLE_SVAR_INPUT(extrudeToolToggle, GLFW_KEY_UNKNOWN,    false)
	DEFINE_TABLE_SVAR_INPUT(extrudeToolHold,   GLFW_KEY_LEFT_SHIFT, false)
END_SVAR_TABLE()

static CNodeToolsSettings s_NodeToolsSettings;
DEFINE_SETTINGS_MENU("Node Tools", s_NodeToolsSettings);

input_t CDragTool::GetToggleInput() { return s_NodeToolsSettings.dragToolToggle; }
input_t CDragTool::GetHoldInput() { return s_NodeToolsSettings.dragToolHold; }


input_t CExtrudeTool::GetToggleInput() { return s_NodeToolsSettings.extrudeToolToggle; }
input_t CExtrudeTool::GetHoldInput() { return s_NodeToolsSettings.extrudeToolHold; }
