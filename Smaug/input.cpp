#include "input.h"
#include "smaugapp.h"


#define PRESSED 0x1
#define JUST_PRESSED 0x2
struct buttonState_t
{
	char state;
	bool Pressed() { return state & PRESSED; }
	bool JustPressed() { return state & JUST_PRESSED; }
};

static buttonState_t m_keyboardButtons[512];
static buttonState_t s_mouseButtons[8];

CInputManager& Input()
{
	static CInputManager s_input;
	return s_input;
}

CInputManager::CInputManager()
{
	memset(s_mouseButtons, 0, sizeof(s_mouseButtons));
	memset(m_keyboardButtons,  0, sizeof(m_keyboardButtons));

}



bool CInputManager::IsDown(input_t in)
{
	if (in.isMouseButton)
		return glfwGetMouseButton(GetApp().GetWindow(), in.code) == GLFW_PRESS;
	else
		return glfwGetKey(GetApp().GetWindow(), in.code) == GLFW_PRESS;
}

bool CInputManager::Clicked(input_t in)
{
	if (in.isMouseButton)
		return s_mouseButtons[in.code].JustPressed();
	else
		return m_keyboardButtons[in.code].JustPressed();
}

float CInputManager::Axis(input_t up, input_t down)
{
	return IsDown(up) - (int)IsDown(down);
}

void CInputManager::SetInput(input_t in, int state)
{
	buttonState_t* s = 0;
	if (in.isMouseButton)
		s = &s_mouseButtons[in.code];
	else
		s = &m_keyboardButtons[in.code];

	if (state == GLFW_PRESS || state == GLFW_REPEAT)
	{
		if (s->Pressed())
			s->state = PRESSED;
		else
			s->state = PRESSED | JUST_PRESSED;
	}
	else
		s->state = 0;
}

void CInputManager::EndFrame()
{
	// Mask off our just pressed stuff
	for(int i = 0; i < sizeof(s_mouseButtons) / sizeof(buttonState_t); i++)
		s_mouseButtons[i].state &= PRESSED;
	for (int i = 0; i < sizeof(m_keyboardButtons) / sizeof(buttonState_t); i++)
		m_keyboardButtons[i].state &= PRESSED;
}


#define KEY(name) {GLFW_KEY_##name, #name, false},
#define MOUSE(name) {GLFW_MOUSE_BUTTON_##name, "MOUSE" #name, true},
static inputName_t s_inputNameUnknown = { GLFW_KEY_UNKNOWN, "UNKNOWN", false };

// 32 to 96 inclusively
static inputName_t s_inputNames[] =
{
	MOUSE(1)
	MOUSE(2)
	MOUSE(3)
	MOUSE(4)
	MOUSE(5)
	MOUSE(6)
	MOUSE(7)
	MOUSE(8)

	MOUSE(LEFT)
	MOUSE(MIDDLE)
	MOUSE(RIGHT)

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
#undef MOUSE


inputName_t* GetAllInputs(size_t* length)
{
	if (length)
	{
		*length = sizeof(s_inputNames) / sizeof(inputName_t);
	}
	return s_inputNames;
}

input_t InputFromName(const char* name)
{
	// Terrible. I hope this doesn't have to run often...
	for (int i = 0; i < sizeof(s_inputNames) / sizeof(inputName_t); i++)
	{
		if (strcmp(name, s_inputNames[i].name) == 0)
			return { s_inputNames[i].id, s_inputNames[i].isMouseButton };
	}

	// No clue. Return unknown.
	return { GLFW_KEY_UNKNOWN, false };
}

const char* InputToName(input_t in)
{
	if (in.code > GLFW_KEY_UNKNOWN && in.code <= GLFW_KEY_LAST)
	{
		for (int i = 0; i < sizeof(s_inputNames) / sizeof(inputName_t); i++)
		{
			if (s_inputNames[i].id == in.code && in.isMouseButton == s_inputNames[i].isMouseButton)
				return s_inputNames[i].name;
		}
	}

	// No clue.
	return s_inputNameUnknown.name;
}
