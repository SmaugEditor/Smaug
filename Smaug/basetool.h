#pragma once

#include <GLFW/glfw3.h>

class CBaseTool
{
public:

	virtual const char* GetName() = 0;
	virtual int GetToggleKey() = 0;
	virtual int GetTempKey() = 0;


	virtual void Enable() = 0;
	virtual void Update() = 0;
	virtual void Disable() = 0;

};

class CDragTool : public CBaseTool
{
public:

	virtual const char* GetName() { return "Drag"; }
	virtual int GetToggleKey() { return GLFW_KEY_G; }
	virtual int GetTempKey() { return 0; }

	virtual void Enable();
	virtual void Update() {  }
	virtual void Disable();

};

class CExtrudeTool : public CBaseTool
{
public:

	virtual const char* GetName() { return "Extrude"; }
	virtual int GetToggleKey() { return 0; }
	virtual int GetTempKey() { return GLFW_KEY_LEFT_SHIFT; }

	virtual void Enable();
	virtual void Update()
	{

	}
	virtual void Disable();

};
