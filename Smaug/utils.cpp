#include "utils.h"
#include <bigg.hpp>

bool IsPointOnLine2D(glm::vec3 point1, glm::vec3 point2, glm::vec3 mouse, float range)
{
	// Top point is always point1
	if (point2.z > point1.z)
	{
		glm::vec3 temp = point1;
		point1 = point2;
		point2 = temp;
	}
	// If the Zs are equal, then the left point is always point1
	else if (point2.z == point1.z && point2.x < point1.x)
	{
		glm::vec3 temp = point1;
		point1 = point2;
		point2 = temp;
	}

	// Is the line is straight up?
	if (point1.x == point2.x)
	{
		return abs(mouse.x - point1.x) < range // Is within the width bounds?
			&& mouse.z <= point1.z  // Is not higher than
			&& mouse.z >= point2.z; // Is not lower than
	}

	// Is the line straight sideways?
	if (point1.z == point2.z)
	{
		return abs(mouse.z - point1.z) < range // Is within the height bounds?
			&& mouse.x >= point1.x  // Is not left of
			&& mouse.x <= point2.x; // Is not right of
	}

	float slope = (point1.z - point2.z) / (point1.x - point2.x);
	float perp = -1 / slope;

	float perpAngle = atan(perp);
	glm::vec3 perpOffset = glm::vec3(range * cos(perpAngle), 0, range * sin(perpAngle));

	glm::vec3 top = point1;
	glm::vec3 bottom = point2;

	
	if (slope < 0) // Line points down. The perpOffset is flipped
	{
		top -= perpOffset;
		bottom += perpOffset;
	}
	else // Line points up
	{
		top += perpOffset;
		bottom -= perpOffset;
	}

	// Bottom Slope
	if ((mouse.x - bottom.x) * slope + bottom.z < mouse.z)
		return false;

	// Top Slope
	if ((mouse.x - top.x) * slope + top.z > mouse.z)
		return false;

	// Top Perp
	if ((mouse.x - top.x) * perp + top.z < mouse.z)
		return false;

	// Bottom Perp
	if ((mouse.x - bottom.x) * perp + bottom.z > mouse.z)
		return false;

	return true;
}

bool IsPointNearPoint2D(glm::vec3 point1, glm::vec3 point2, float range)
{
	// Ignore Y
	point1.y = 0;
	point2.y = 0;

	return glm::length(point2 - point1) <= range;
}

bgfx::ProgramHandle LoadShader(const char* fragment, const char* vertex)
{
	// Static so we don't reinitiate this
	static const char** shaderPaths = new const char* [10]
	{
		nullptr,		  // No Renderer
		"shaders/dx9/",   // Direct3D9
		"shaders/dx11/",  // Direct3D11
		"shaders/dx11/",  // Direct3D12
		nullptr,		  // Gnm
		nullptr,		  // Metal
		nullptr,		  // Nvm
		"shaders/glsl/",  // OpenGL
		nullptr,		  // OpenGL ES
		"shaders/spirv/", // Vulkan
	};

	int type = bgfx::getRendererType();

	// Out of bounds. Return an invalid handle
	if (type >= bgfx::RendererType::Count || type < 0)
	{
		return BGFX_INVALID_HANDLE;
	}

	const char* shaderPath = shaderPaths[bgfx::getRendererType()];

	// Unsupported platform. Return invalid handle
	if (!shaderPath)
	{
		return BGFX_INVALID_HANDLE;
	}

	char vsPath[128];
	strcpy(vsPath, shaderPath);
	strcat(vsPath, vertex);

	char fsPath[128];
	strcpy(fsPath, shaderPath);
	strcat(fsPath, fragment);

	return bigg::loadProgram(vsPath, fsPath);
}

static const float s_MathPI = acos(-1);

void Directions(glm::vec3 angles, glm::vec3* forward, glm::vec3* right, glm::vec3* up)
{
	const float pitch = angles.x;
	const float yaw = angles.y;
	const float roll = -angles.z;

	glm::vec3 _forward;
	_forward.x = sin(yaw) * cos(pitch);
	_forward.y = -sin(pitch);
	_forward.z = cos(yaw) * cos(pitch);
	_forward = glm::normalize(_forward);

	if (forward)
		*forward = _forward;


	glm::vec3 _right;
	_right.x = cos(roll) * cos(yaw + s_MathPI);
	_right.y = sin(roll);
	_right.z = -cos(roll) * sin(yaw + s_MathPI);
	_right = glm::normalize(_right);


	if (right)
		*right = _right;

	if (up)
		*up = glm::normalize(glm::cross(_right, _forward));
/*
	if (up)
	{
		glm::vec3 _up;
		_up.x = cos(-(roll + yaw)) * sin(pitch);
		_up.y = cos(-roll) * cos(pitch);
		_up.z = sin(-roll) * cos(yaw) * ((sin(pitch) + 1)/2);
		_up = glm::normalize(_up);

		*up = _up;
	}
	*/
}



#define KEY(name) {GLFW_KEY_##name, #name},
static keyName_t s_keyNameUnknown = { GLFW_KEY_UNKNOWN, "UNKNOWN" };

// 32 to 96 inclusively
static keyName_t s_keyNames[] =
{
	KEY(SPACE)
	KEY(APOSTROPHE)
	KEY(COMMA)
	KEY(MINUS)
	KEY(PERIOD)
	KEY(SLASH)
	KEY(0)
	KEY(1)
	KEY(2)
	KEY(3)
	KEY(4)
	KEY(5)
	KEY(6)
	KEY(7)
	KEY(8)
	KEY(9)
	KEY(SEMICOLON)
	KEY(EQUAL)
	KEY(A)
	KEY(B)
	KEY(C)
	KEY(D)
	KEY(E)
	KEY(F)
	KEY(G)
	KEY(H)
	KEY(I)
	KEY(J)
	KEY(K)
	KEY(L)
	KEY(M)
	KEY(N)
	KEY(O)
	KEY(P)
	KEY(Q)
	KEY(R)
	KEY(S)
	KEY(T)
	KEY(U)
	KEY(V)
	KEY(W)
	KEY(X)
	KEY(Y)
	KEY(Z)
	KEY(LEFT_BRACKET)
	KEY(BACKSLASH)
	KEY(RIGHT_BRACKET)
	KEY(GRAVE_ACCENT)
	KEY(WORLD_1)
	KEY(WORLD_2)
	KEY(ESCAPE)
	KEY(ENTER)
	KEY(TAB)
	KEY(BACKSPACE)
	KEY(INSERT)
	KEY(DELETE)
	KEY(RIGHT)
	KEY(LEFT)
	KEY(DOWN)
	KEY(UP)
	KEY(PAGE_UP)
	KEY(PAGE_DOWN)
	KEY(HOME)
	KEY(END)
	KEY(CAPS_LOCK)
	KEY(SCROLL_LOCK)
	KEY(NUM_LOCK)
	KEY(PRINT_SCREEN)
	KEY(PAUSE)
	KEY(F1)
	KEY(F2)
	KEY(F3)
	KEY(F4)
	KEY(F5)
	KEY(F6)
	KEY(F7)
	KEY(F8)
	KEY(F9)
	KEY(F10)
	KEY(F11)
	KEY(F12)
	KEY(F13)
	KEY(F14)
	KEY(F15)
	KEY(F16)
	KEY(F17)
	KEY(F18)
	KEY(F19)
	KEY(F20)
	KEY(F21)
	KEY(F22)
	KEY(F23)
	KEY(F24)
	KEY(F25)
	KEY(KP_0)
	KEY(KP_1)
	KEY(KP_2)
	KEY(KP_3)
	KEY(KP_4)
	KEY(KP_5)
	KEY(KP_6)
	KEY(KP_7)
	KEY(KP_8)
	KEY(KP_9)
	KEY(KP_DECIMAL)
	KEY(KP_DIVIDE)
	KEY(KP_MULTIPLY)
	KEY(KP_SUBTRACT)
	KEY(KP_ADD)
	KEY(KP_ENTER)
	KEY(KP_EQUAL)
	KEY(LEFT_SHIFT)
	KEY(LEFT_CONTROL)
	KEY(LEFT_ALT)
	KEY(LEFT_SUPER)
	KEY(RIGHT_SHIFT)
	KEY(RIGHT_CONTROL)
	KEY(RIGHT_ALT)
	KEY(RIGHT_SUPER)
	KEY(MENU)
};
#undef KEY

keyName_t* GetAllKeys(size_t* length)
{
	if (length)
	{
		*length = sizeof(s_keyNames) / sizeof(keyName_t);
	}
	return s_keyNames;
}

key_t KeyFromName(const char* name)
{
	// Terrible. I hope this doesn't have to run often...
	for (int i = 0; i < sizeof(s_keyNames) / sizeof(keyName_t); i++)
	{
		if (strcmp(name, s_keyNames[i].name) == 0)
			return { s_keyNames[i].id };
	}

	// No clue. Return unknown.
	return { GLFW_KEY_UNKNOWN };
}

const char* KeyToName(key_t key)
{
	if (key.key > GLFW_KEY_UNKNOWN && key.key <= GLFW_KEY_LAST)
	{
		for (int i = 0; i < sizeof(s_keyNames) / sizeof(keyName_t); i++)
		{
			if (s_keyNames[i].id == key.key)
				return s_keyNames[i].name;
		}
	}

	// No clue.
	return s_keyNameUnknown.name;
}
