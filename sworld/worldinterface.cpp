#include "worldinterface.h"
#include <cstring>

class CDefaultWorldInterface : public IWorldInterface
{
public:
	virtual INodeRenderer* CreateNodeRenderer()
	{
		return nullptr;
	}
	virtual void LookupTextureName(texture_t texture, char* dest, size_t len)
	{
		if(dest)
			strncpy(dest, "NO_WORLD_INTERFACE", len);
	}
	virtual texture_t LookupTextureId(const char* texture)
	{
		return INVALID_TEXTURE;
	}
};

static IWorldInterface* s_worldInterface = new CDefaultWorldInterface;

void SupplyWorldInterface(IWorldInterface* wi)
{
	if (s_worldInterface)
		delete s_worldInterface;
	s_worldInterface = wi;
}

IWorldInterface* WorldInterface()
{
	return s_worldInterface;
}
