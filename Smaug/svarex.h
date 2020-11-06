#pragma once
#include "svar.h"
#include "utils.h"
#include "glm/vec3.hpp"

// Extra types for things like glm::vec3


// Check how long floats can be at some point
BEGIN_SVAR_TYPE_IMPLEMENT(glm::vec3)
	IMPL_FORMAT_TOSTRING_EX(128, "%f %f %f", m_data.x, m_data.y, m_data.z)
	virtual void FromString(char* str)
	{
		char* endptr;
		m_data.x = strtof(str, &endptr);
		m_data.y = strtof(endptr, &endptr);
		m_data.z = strtof(endptr, nullptr);
	}
END_SVAR_TYPE_IMPLEMENT()



#define DEFINE_TABLE_SVAR_KEY(name, value) CSVar<key_t> name{#name, {value}, this};
BEGIN_SVAR_TYPE_IMPLEMENT(key_t)
	virtual char* ToString()
	{
		const char* keyName = KeyToName(m_data);
		size_t len = strlen(keyName);
		char* str = new char[len + 1];
		strncpy(str, keyName, len + 1);
		return str;
	}
	virtual void FromString(char* str)
	{
		m_data = KeyFromName(str);
	}
END_SVAR_TYPE_IMPLEMENT()
