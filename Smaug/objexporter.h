#pragma once
#include "baseexporter.h"

class COBJExporter : public CBaseExporter
{
public:
	virtual char* Export(CWorldEditor* world) override;
};

