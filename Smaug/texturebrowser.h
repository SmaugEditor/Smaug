#pragma once

#include "texturemanager.h"
#include <vector>
#include <string>
#include <tuple>

class CTextureBrowser
{
public:
	CTextureBrowser();
	void Show();
	texture_t SelectedTexture() { return m_selectedTexture; }
private:
	std::vector<std::tuple<std::string, texture_t>> m_textures;
	texture_t m_selectedTexture;
};
