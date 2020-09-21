#pragma once

#include <bgfx/bgfx.h>
#include <map>
#include <string>

class CTextureManager
{
public:
	bgfx::TextureHandle LoadTexture(const char* path);
private:
	// We map paths to textures as we load them so that we don't reload them later
	std::map<std::string, bgfx::TextureHandle> m_textureMap;
};

CTextureManager& GetTextureManager();