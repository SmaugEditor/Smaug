#pragma once
#include "shared.h"

// A Complete implementation of IWorldInterface must be complete for full functionality of the world engine

class CNode;
class INodeRenderer
{
public:
	virtual void SetOwner(CNode* node) = 0;
	virtual void Rebuild() = 0;
	virtual void Destroy() = 0;
};

class IWorldInterface
{
public:
	virtual INodeRenderer* CreateNodeRenderer() = 0;
	virtual void LookupTextureName(texture_t texture, char* dest, size_t len) = 0;
	virtual texture_t LookupTextureId(const char* texture) = 0;
};

IWorldInterface* WorldInterface();

void SupplyWorldInterface(IWorldInterface* wi);