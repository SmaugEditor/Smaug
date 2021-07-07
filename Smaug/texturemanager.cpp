#include "texturemanager.h"
#include "log.h"
#include "filesystem.h"

#include <lodepng.h>

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

	unsigned int width, height;
	std::vector<unsigned char> pixels;

	{

		size_t len;
		const unsigned char* buf = (const unsigned char*)filesystem::LoadFile(path, len);
		if (!buf)
		{
			Log::TWarn("Failed to load image %s: File not found\n", path);
			delete[] buf;
			return ErrorTexture();
		}

		if (len < 10)
		{
			Log::TWarn("Failed to load image %s: file is too small to be a png\n", path);
			delete[] buf;
			return ErrorTexture();
		}

		// LFS check
		// An LFS meta file starts with "version"
		// if (strcmp(contents, "version") == 0)
		if (*(uint64_t*)buf == *(uint64_t*)"version ") // Space is here so that we don't try to test an unexpected \0
		{
			Log::TWarn("Failed to load image %s: LFS detected\n", path);
			Log::TWarn("See the Getting Started section of the README for more info\n");
			delete[] buf;
			return ErrorTexture();
		}

		unsigned int error = lodepng::decode(pixels, width, height, buf, len);

		if (error)
		{
			const char* errorMessage = lodepng_error_text(error);
			Log::TWarn("Failed to load image %s: Lodepng error - %s\n", path, errorMessage);

			delete[] buf;
			return ErrorTexture();
		}

		delete[] buf;
	}

	if (!pixels.size() || width == 0 || height == 0)
	{
		// Oh, no. We failed to load the image...
		Log::TWarn("Failed to load image %s: Empty image\n", path);
		return ErrorTexture();
	}


	// If we use a reference here, we'd have to maintain the pixel data ourselves. This just makes it easier
	const bgfx::Memory* mem = bgfx::alloc(pixels.size());
	memcpy(mem->data, pixels.data(), pixels.size());

	// Lode png puts everything in rgba8 unless requested otherwise
	bgfx::TextureHandle textureHandle = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::Enum::RGBA8, 0, mem);

	// Put this in our map so we can grab it later instead of loading it again.
	m_textureMap.insert({ path, textureHandle });


	Log::TPrint("Loaded image %s\n", path);
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
