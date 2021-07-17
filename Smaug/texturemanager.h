#pragma once

#include <bgfx/bgfx.h>
#include <map>
#include <string>

typedef bgfx::TextureHandle texture_t;

class CTextureManager
{
public:
	texture_t LoadTexture(const char* path);
	texture_t ErrorTexture();
private:
	// We map paths to textures as we load them so that we don't reload them later
	std::map<std::string, texture_t> m_textureMap;
};

CTextureManager& TextureManager();