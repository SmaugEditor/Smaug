#pragma once

#include <bgfx/bgfx.h>
#include <vector>
#include <string>
#include <tuple>

class CTextureBrowser
{
public:
	CTextureBrowser();
	void Show();

private:
	std::vector<std::tuple<std::string, bgfx::TextureHandle>> m_textures;
};

CTextureBrowser& TextureBrowser();