#include "interfaceimpl.h"
#include "meshrenderer.h"
#include "filesystem.h"
#include <GLFW/glfw3.h>
#include <basicdraw.h>
#include <smaugapp.h>
#include <worldrenderer.h>

INodeRenderer* CEngineInterface::CreateNodeRenderer()
{
	return new CMeshRenderer;
}

void CEngineInterface::LookupTextureName(texture_t texture, char* dest, size_t len)
{
	if (dest)
		TextureManager().TextureName(texture, dest, len);
}

texture_t CEngineInterface::LoadTexture(const char* texture)
{
	return TextureManager().LoadTexture(texture);
}

float CEngineInterface::Time()
{
	return glfwGetTime();
}

char* CEngineInterface::ReadFile(const char* path, size_t& fileLen)
{
	return filesystem::LoadFile(path, fileLen);
}

void CEngineInterface::SaveFile(const char* path, char* buf, size_t len)
{
	filesystem::SaveFile(path, buf);
}

char* CEngineInterface::ReadFilePrompt(size_t& fileLen, char* pathOut, size_t destLen)
{
	return filesystem::LoadFileWithDialog(fileLen, "");
}

bool CEngineInterface::SaveFilePrompt(char* buf, size_t len)
{
	return filesystem::SaveFileWithDialog(buf, "");
}

void CEngineInterface::DrawLine(glm::vec3 start, glm::vec3 end, float width, glm::vec3 color)
{
	BasicDraw().Line(start, end, color, width);
}

IModel* CEngineInterface::LoadModel(const char* path)
{
	return ModelManager().LoadModel(path);
}

void CEngineInterface::LockMouse(bool locked)
{
	GetApp().SetMouseLock(locked);
}

renderTarget_t CEngineInterface::CreateRenderTarget(uint32_t width, uint32_t height)
{
	bgfx::TextureHandle fbColorTexture = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::RGBA16F, (uint64_t(1) << BGFX_TEXTURE_RT_MSAA_SHIFT) | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP);

	uint64_t textureFlags = BGFX_TEXTURE_RT_WRITE_ONLY;
	bgfx::TextureFormat::Enum depthFormat =
		bgfx::isTextureValid(0, false, 1, bgfx::TextureFormat::D16, textureFlags) ? bgfx::TextureFormat::D16
		: bgfx::isTextureValid(0, false, 1, bgfx::TextureFormat::D24S8, textureFlags) ? bgfx::TextureFormat::D24S8
		: bgfx::TextureFormat::D32;

	bgfx::TextureHandle fbDepthTexture = bgfx::createTexture2D(width, height, false, 1, depthFormat, textureFlags);

	bgfx::TextureHandle pair[2] = { fbColorTexture, fbDepthTexture };
	bgfx::FrameBufferHandle framebuffer = bgfx::createFrameBuffer(2, pair, true);
	
	uint16_t id = (uint16_t)m_renderTargets.size() + 1;
	m_renderTargets.push_back({ id, fbColorTexture, fbDepthTexture, framebuffer, (uint16_t)width, (uint16_t)height });
	return reinterpret_cast<renderTarget_t>(id);
}

void CEngineInterface::ClearColor(renderTarget_t rt, uint32_t color)
{
	bgfx::setViewClear(reinterpret_cast<uint16_t>(rt), BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, color);
}

void CEngineInterface::BeginView(renderTarget_t rt)
{
	uint16_t id = reinterpret_cast<uint16_t>(rt);
	
	auto& view = m_renderTargets[id-1];
	ModelManager().SetView(id);
	m_curView = id;
	if (id != 0)
		bgfx::setViewFrameBuffer(id, view.framebuffer);
	bgfx::setViewRect(id, 0, 0, view.width, view.height);

	bgfx::touch(id);
}

void CEngineInterface::EndView(renderTarget_t rt)
{
}

void CEngineInterface::SetViewMatrix(glm::mat4& view, glm::mat4& proj)
{
	memcpy(&m_view, &view, sizeof(glm::mat4));
	memcpy(&m_proj, &proj, sizeof(glm::mat4));
	bgfx::setViewTransform(m_curView, &m_view[0][0], &m_proj[0][0]);
}

void CEngineInterface::DrawWorld3D()
{
	GetWorldRenderer().Draw3D(m_curView, Shader::WORLD_PREVIEW_SHADER);
}

void CEngineInterface::DrawWorld2D()
{
	GetWorldRenderer().Draw2D(m_curView, Shader::WORLD_PREVIEW_SHADER);
}

texture_t CEngineInterface::TextureFromRenderTarget(renderTarget_t rt)
{
	return reinterpret_cast<texture_t>(bgfx::getTexture(m_renderTargets[reinterpret_cast<uint16_t>(rt)-1].framebuffer).idx);
}
