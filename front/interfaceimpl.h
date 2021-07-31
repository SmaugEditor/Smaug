#pragma once
#include "editorinterface.h"
#include <bgfx/bgfx.h>
#include <vector>
struct engineRT_t
{
	uint16_t                viewId;
	bgfx::TextureHandle     fbColorTexture;
	bgfx::TextureHandle     fbDepthTexture;
	bgfx::FrameBufferHandle framebuffer;
	uint16_t                width;
	uint16_t                height;
};

// Connect up to the editor
class CEngineInterface : public IEngineInterface
{
public:
	virtual INodeRenderer* CreateNodeRenderer();
	virtual void LookupTextureName(texture_t texture, char* dest, size_t len);
	virtual texture_t LoadTexture(const char* texture) override;
	virtual float Time() override;
	virtual char* ReadFile(const char* path, size_t& fileLen) override;
	virtual void SaveFile(const char* path, char* buf, size_t len) override;
	virtual char* ReadFilePrompt(size_t& fileLen, char* pathOut, size_t destLen) override;
	virtual bool SaveFilePrompt(char* buf, size_t len) override;
	virtual void DrawLine(glm::vec3 start, glm::vec3 end, float width, glm::vec3 color) override;
	virtual IModel* LoadModel(const char* path) override;
	virtual void LockMouse(bool locked) override;
	virtual renderTarget_t CreateRenderTarget(uint32_t width, uint32_t height) override;
	virtual void ClearColor(renderTarget_t rt, uint32_t color) override;
	virtual void BeginView(renderTarget_t rt) override;
	virtual void EndView(renderTarget_t rt) override;
	virtual void SetViewMatrix(glm::mat4& view, glm::mat4& proj) override;
	virtual void DrawWorld3D() override;
	virtual void DrawWorld2D() override;
	virtual void DrawGrid(CTransform& transform, int scale) override;
	virtual texture_t TextureFromRenderTarget(renderTarget_t rt) override;

	glm::mat4 m_view;
	glm::mat4 m_proj;

	std::vector<engineRT_t> m_renderTargets;
	uint16_t m_curView;
};