#include "interfaceimpl.h"
#include "meshrenderer.h"

INodeRenderer* CWorldInterface::CreateNodeRenderer()
{
	return new CMeshRenderer;
}

void CWorldInterface::LookupTextureName(texture_t texture, char* dest, size_t len)
{
	if (dest)
		TextureManager().TextureName(texture, dest, len);
}

texture_t CWorldInterface::LookupTextureId(const char* texture)
{
	return TextureManager().LoadTexture(texture);
}
