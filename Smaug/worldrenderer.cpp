#include "worldrenderer.h"
#include "worldeditor.h"
#include "utils.h"

#include <bgfx/bgfx.h>
#include <glm/gtc/matrix_transform.hpp>

void CWorldRenderer::Init()
{
}

void CWorldRenderer::Draw2D(bgfx::ViewId viewId, Shader shader)
{
	bgfx::ProgramHandle shaderProgram = ShaderManager().GetShaderProgram(shader);
	CWorldEditor& world = GetWorldEditor();
	for (auto p : world.m_nodes)
	{
		CNode* node = p.second;
		if (node->m_renderData.Empty())
			continue;

		// Set the color
		// Precompute this?
		ShaderManager().SetColor(glm::vec4(nodeColor(node), 0.85f));
		bgfx::setState(
			  BGFX_STATE_CULL_CW
			| BGFX_STATE_WRITE_RGB
			| BGFX_STATE_WRITE_A
			| BGFX_STATE_DEPTH_TEST_ALWAYS
			| BGFX_STATE_MSAA
			| BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_ONE)
			| BGFX_STATE_BLEND_INDEPENDENT
			, BGFX_STATE_BLEND_FUNC_RT_1(BGFX_STATE_BLEND_ZERO, BGFX_STATE_BLEND_SRC_COLOR)
		);
		node->m_renderData.Render(viewId, shaderProgram);
	}
}

void CWorldRenderer::Draw3D(bgfx::ViewId viewId, Shader shader)
{
	bgfx::ProgramHandle shaderProgram = ShaderManager().GetShaderProgram(shader);
	CWorldEditor& world = GetWorldEditor();
	for (auto p : GetWorldEditor().m_nodes)
	{
		CNode* node = p.second;
		if (node->m_renderData.Empty())
			continue;

		if(node->IsVisible())
		{

			// Set the color
			// Precompute this?
			ShaderManager().SetColor(glm::vec4(nodeColor(node), 1.0f));
			node->m_renderData.Render(viewId, shaderProgram);
		}
	}
}

glm::vec3 nodeColor(CNode* node)
{
	glm::vec3 origin = node->m_mesh.origin;
	float mag = glm::length(origin);
	origin = glm::normalize(origin);
	float theta = atan2(origin.z, origin.x) + PI;
	float saturation = (sin(mag * PI * 2 / 24.0f) + 1.0f) / 2.0f * 0.75f + 0.25f;
	return colorHSV(theta, saturation, 1.0f);
}

CWorldRenderer& GetWorldRenderer()
{
	static CWorldRenderer worldRenderer;
	return worldRenderer;
}
