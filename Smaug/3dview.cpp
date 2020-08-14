#include "3dview.h"

#include <bigg.hpp>
#include <bx/string.h>
#include "worldrenderer.h"

#include <glm/gtc/matrix_transform.hpp>


void C3DView::Init(bgfx::ViewId viewId, int width, int height, uint32_t clearColor)
{
	CBaseView::Init(viewId, width, height, clearColor);

	PosColorVertex::init();

	char vsName[32];
	char fsName[32];

	const char* shaderPath = "???";

	switch (bgfx::getRendererType())
	{
	case bgfx::RendererType::Noop:
	case bgfx::RendererType::Direct3D9:  shaderPath = "shaders/dx9/";   break;
	case bgfx::RendererType::Direct3D11:
	case bgfx::RendererType::Direct3D12: shaderPath = "shaders/dx11/";  break;
	case bgfx::RendererType::Gnm:                                       break;
	case bgfx::RendererType::Metal:      shaderPath = "shaders/metal/"; break;
	case bgfx::RendererType::OpenGL:     shaderPath = "shaders/glsl/";  break;
	case bgfx::RendererType::OpenGLES:   shaderPath = "shaders/essl/";  break;
	case bgfx::RendererType::Vulkan:     shaderPath = "shaders/spirv/"; break;
	case bgfx::RendererType::Count:                                     break;
	}

	bx::strCopy(vsName, BX_COUNTOF(vsName), shaderPath);
	bx::strCat(vsName, BX_COUNTOF(vsName), "vs_cubes.bin");

	bx::strCopy(fsName, BX_COUNTOF(fsName), shaderPath);
	bx::strCat(fsName, BX_COUNTOF(fsName), "fs_cubes.bin");

	m_hShaderProgram = bigg::loadProgram(vsName, fsName);
	m_hVertexBuf = bgfx::createVertexBuffer(bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices)), PosColorVertex::ms_layout);
	m_hIndexBuf = bgfx::createIndexBuffer(bgfx::makeRef(s_cubeTriList, sizeof(s_cubeTriList)));

	m_flTime = 0.0f;
}

void C3DView::Draw(float dt)
{
	CBaseView::Draw(dt);

	m_flTime += dt;

	glm::mat4 view = glm::lookAt(glm::vec3(10.0f, 85.0f, 40.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 proj = glm::perspective(glm::radians(60.0f), float(m_width) / m_height, 0.1f, 800.0f);
	bgfx::setViewTransform(m_viewId, &view[0][0], &proj[0][0]);

	GetWorldRenderer().Draw(m_viewId, m_hShaderProgram);
}