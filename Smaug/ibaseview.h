#pragma once
#include <bgfx/bgfx.h>
#include <imgui.h>

class IBaseView
{
public:
	virtual void Init(bgfx::ViewId viewId, int width, int height, uint32_t clearColor) = 0;
	virtual void Update(float dt) = 0;
	virtual ImTextureID GetImTextureID() = 0;
};
