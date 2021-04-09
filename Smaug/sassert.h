#pragma once

bool smaugAssert(bool condition, const char* expression, int line, const char* file, const char* function);
#ifdef _DEBUG
#define SASSERT(cond) smaugAssert(cond, #cond, __LINE__, __FILE__, __FUNCTION__)
#else
// We still eval! Just incase some odd ball code exists that says something like i++
#define SASSERT(cond) (cond)
#endif