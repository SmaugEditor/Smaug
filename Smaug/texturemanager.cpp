#include "texturemanager.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


bgfx::TextureHandle CTextureManager::LoadTexture(const char* path)
{
	auto textureSearch = m_textureMap.find(path);
	if (textureSearch != m_textureMap.end())
	{
		// Turns out, we already have this loaded.
		// Let's just return it then
		return textureSearch->second;
	}

	// We don't have it loaded... Guess we'll have to grab it then!

	// Comp is how many components each pixel is made up of
	int width, height, comp;

	stbi_uc* image = stbi_load(path, &width, &height, &comp, 0);
		
	// Did we actually get the image?
	if (image)
	{
		size_t size = (size_t)width * height * comp * sizeof(stbi_uc);

		// If we use a reference here, we'd have to maintain the pixel data ourselves. This just makes it easier
		const bgfx::Memory* mem = bgfx::alloc(size);
		memcpy(mem->data, image, size);
		delete[] image;

		// Figure out what kinda foramt this uses
		bgfx::TextureFormat::Enum format = bgfx::TextureFormat::Enum::Unknown;
		switch (comp)
		{
		case 1:
			format = bgfx::TextureFormat::Enum::R8; // Wrong. Should be Gray
			break;
		case 2:
			format = bgfx::TextureFormat::Enum::RG8; // Wrong. Should be Gray Alpha
			break;
		case 3:
			format = bgfx::TextureFormat::Enum::RGB8;
			break;
		case 4:
			format = bgfx::TextureFormat::Enum::RGBA8;
			break;
		}


		bgfx::TextureHandle textureHandle = bgfx::createTexture2D(width, height, false, 1, format, 0, mem);

		// Put this in our map so we can grab it later instead of loading it again.
		m_textureMap.insert({ path, textureHandle });

		return textureHandle;
	}

	// Oh, no. We failed to load the image...
	printf("[ERROR] Failed to load image %s\n", path);
	return BGFX_INVALID_HANDLE;
}

CTextureManager& TextureManager()
{
	static CTextureManager s_textureManager;
	return s_textureManager;
}
