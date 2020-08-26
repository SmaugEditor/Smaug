#pragma once

class CWorldEditor;
class CBaseExporter
{
public:
	virtual char* Export(CWorldEditor* world) = 0;
};

