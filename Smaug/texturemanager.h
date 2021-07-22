#pragma once

#include "utils.h"

#include <bgfx/bgfx.h>
#include <map>
#include <string>


class CTextureManager
{
public:
	texture_t LoadTexture(const char* path);
	texture_t ErrorTexture();

	void TextureName(texture_t id, char* dest, size_t len);
private:
	// We map paths to textures as we load them so that we don't reload them later
	std::map<std::string, texture_t> m_textureMap;
};

CTextureManager& TextureManager();