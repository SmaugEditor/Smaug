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

	// Draw this node at its mesh origin
	virtual void Draw() = 0;
};

class IWorldInterface
{
public:
	// Does not need to render in headless
	virtual INodeRenderer* CreateNodeRenderer() = 0;

	// For the mesher, this does not need to actually be a texture, could just be a number id
	// BUT when not headless, THIS NEEDS TO BE VALID
	virtual void LookupTextureName(texture_t texture, char* dest, size_t len) = 0;
	virtual texture_t LoadTexture(const char* texture) = 0;
};

IWorldInterface* WorldInterface();

void SupplyWorldInterface(IWorldInterface* wi);