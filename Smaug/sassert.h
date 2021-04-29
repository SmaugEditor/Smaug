#pragma once

// On failure, logs and pops up a message box
bool smaugAssert(bool condition, const char* expression, int line, const char* file, const char* function);

// On failure, logs only
bool smaugAssertSilent(bool condition, const char* expression, int line, const char* file, const char* function);

#ifdef _DEBUG
#define SASSERT(cond) smaugAssert(cond, #cond, __LINE__, __FILE__, __FUNCTION__)
#define SASSERT_S(cond) smaugAssertSilent(cond, #cond, __LINE__, __FILE__, __FUNCTION__)
#else
// We still eval! Just incase some odd ball code exists that says something like i++
#define SASSERT(cond) (cond)
#define SASSERT_S(cond) (cond)
#endif