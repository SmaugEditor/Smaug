#pragma once

#include <cstddef>

struct input_t
{
	int code;
	bool isMouseButton;

};

struct inputName_t
{
	int id;
	const char* name;
	bool isMouseButton;
};

inputName_t* GetAllInputs(size_t* length);
input_t InputFromName(const char* name);
const char* InputToName(input_t in);


// Normally I'd just namespace this, but as class, we can turn it into an interface down the line for easier integration into other applications
class CInputManager
{
public:
	CInputManager();

	bool IsDown(input_t in);
	bool Clicked(input_t in);
	float Axis(input_t up, input_t down);
	void SetInput(input_t in, int state);
	void EndFrame();
};

CInputManager& Input();
