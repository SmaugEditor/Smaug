#pragma once

#include <bgfx/bgfx.h>
#include <map>
#include <string>

// Dummy stuff so when we rewire this to be bgfx independent, it's easier
typedef uint16_t texture_t;
#define INVALID_TEXTURE bgfx::kInvalidHandle

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