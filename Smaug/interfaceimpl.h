#pragma once
#include "worldinterface.h"

// Connect up to the world
class CWorldInterface : public IWorldInterface
{
public:
	virtual INodeRenderer* CreateNodeRenderer();
	virtual void LookupTextureName(texture_t texture, char* dest, size_t len);
	virtual texture_t LookupTextureId(const char* texture);
};