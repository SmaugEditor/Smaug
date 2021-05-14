#include "texturemanager.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


const static uint8_t s_errorTexture[] =
{
	255, 000, 000,   255, 255, 255,
	255, 255, 255,   255, 000, 000
};

class CErrorTexture
{
public:
	CErrorTexture()
	{
		const bgfx::Memory* mem = bgfx::alloc(sizeof(s_errorTexture));
		memcpy(mem->data, s_errorTexture, sizeof(s_errorTexture));
		m_textureHandle = bgfx::createTexture2D(2, 2, false, 1, bgfx::TextureFormat::Enum::RGB8, BGFX_TEXTURE_NONE | BGFX_SAMPLER_POINT, mem);
	}

	bgfx::TextureHandle m_textureHandle;
};

static bgfx::TextureHandle GetErrorTexture()
{
	static CErrorTexture s_errorT;
	return s_errorT.m_textureHandle;
}

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
	stbi_uc* image;

	// An unrolled version of stbi_load
	// lets us do better error reporting
	{
		FILE* f = stbi__fopen(path, "rb");
		if (!f)
		{
			printf("[Texture Manager] Failed to load image %s: File not found\n", path);
			return ErrorTexture();
		}

		// LFS check
		char contents[8];
		fgets(contents, sizeof(contents), f);
		// An LFS meta file starts with "version"
		// if (strcmp(contents, "version") == 0)
		if (*(uint64_t*)contents == *(uint64_t*)"version")
		{
			printf("[Texture Manager] Failed to load image %s: LFS detected\n", path);
			printf("[Texture Manager] See the Getting Started section of the README for more info\n");
			return ErrorTexture();
		}

		rewind(f);
		image = stbi_load_from_file(f, &width, &height, &comp, 0);
		fclose(f);
	}

	if (!image)
	{
		// Oh, no. We failed to load the image...
		printf("[Texture Manager] Failed to load image %s: Corrupt image\n", path);
		return ErrorTexture();
	}

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


	printf("[Texture Manager] Loaded image %s\n", path);
	return textureHandle;
}

bgfx::TextureHandle CTextureManager::ErrorTexture()
{
	return GetErrorTexture();
}

CTextureManager& TextureManager()
{
	static CTextureManager s_textureManager;
	return s_textureManager;
}
