#pragma once
#include <cstdint>
#include <imgui.h>

class IBaseView
{
public:
	virtual void Init(uint16_t viewId, int width, int height, uint32_t clearColor) = 0;
	virtual void Draw(float dt) = 0;
	virtual void Update(float dt, float mx, float my) = 0; // Having mouse input input here might be odd, but I'm not sure yet
	virtual ImTextureID GetImTextureID() = 0;
};
