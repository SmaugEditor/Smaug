#include "selectedview.h"

#include "worldrenderer.h"
#include "shadermanager.h"
#include "actionmanager.h"

#include <glm/gtc/matrix_transform.hpp>


void CSelectedView::Init(bgfx::ViewId viewId, int width, int height, uint32_t clearColor)
{
	CBaseView::Init(viewId, width, height, clearColor);
}

void CSelectedView::Draw(float dt)
{
	CBaseView::Draw(dt);

	glm::mat4 view = glm::lookAt(glm::vec3(10.0f, 85.0f, 40.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 proj = glm::perspective(glm::radians(60.0f), float(m_width) / m_height, 0.1f, 800.0f);
	bgfx::setViewTransform(m_viewId, &view[0][0], &proj[0][0]);

	CNode* selectedNode = GetActionManager().selectedNode;
	if (selectedNode)
	{
		selectedNode->m_renderData.Draw(glm::vec3(0,0,0));
		bgfx::submit(m_viewId, ShaderManager::GetShaderProgram(Shader::WORLD_PREVIEW_SHADER));
	}
}