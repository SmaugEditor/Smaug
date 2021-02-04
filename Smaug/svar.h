#pragma once
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <typeinfo>

// CSVars are used for storing settings and saved info.
// These can be easily saved to and loaded from files.


#define BEGIN_SVAR_TABLE(name)        \
class name : public CSVarTable        \
{                                     \
public:                               \
	name(){ m_tableName = #name; }    \
	virtual CSVarTable* New() { return new name(); }


#define END_SVAR_TABLE() };

#define DEFINE_TABLE_SVAR(name, value) CSVar<decltype(value)> name{#name, value, this};
// Make the pain stop. This strips off the ref so that we can strip off const, but, because we stripped off the ref, we have to add a pointer back. Wild
#define DEFINE_TABLE_SVAR_ARRAY(name, value) CSVar<std::add_pointer<std::remove_const<std::remove_reference<decltype(*value)>::type>::type>::type> name{#name, value, this};
#define DEFINE_TABLE_SVAR_TYPED(type, name, value) CSVar<type> name{#name, value, this};

class CSVarTable;

class ISVar;

template<typename T>
class internal_SVar;

template<typename T>
class CSVar : public internal_SVar<T>
{
public:
	/*
	These are all defined later on.

	CSVar(const char* name, T value);
	CSVar(const char* name, T value, CSVarTable* table);

	void SetName(const char* name);
	const char* GetName();
	void* GetData();

	void  SetValue(T value);
	T     GetValue();

	char* ToString();
	void  FromString(char* str);
	
	*/
};



class KeyValue;
// Move elsewhere?
class CSVarTable
{
public:
	void Register(ISVar* var) { m_varTable.push_back(var); }
	char* ToString();
	void FromString(char* str);
	void AddToKV(KeyValue* kv);
	void FromKV(KeyValue* kv);
	virtual CSVarTable* MakeCopy();
	virtual CSVarTable* New() { return new CSVarTable(); }

	std::vector<ISVar*> m_varTable;
	const char* m_tableName = 0;
};



class ISVar
{
public:
	virtual void SetName(const char* name) = 0;
	virtual const char* GetName() = 0;
	virtual void* GetData() = 0;
	virtual void  SetData(void* data) = 0;

	virtual char* ToString() = 0;
	virtual void  FromString(char* str) = 0;

	virtual const type_info& GetTypeInfo() = 0;

	virtual ISVar* MakeCopy() = 0;
};


//
// All below is just for making SVars work. Ignore unless you need to edit SVar.
//

template<typename T>
class internal_SVar : public ISVar
{
public:
	virtual void Init(const char* name, T value) { SetName(name); SetValue(value); }
	virtual void Init(const char* name, T value, CSVarTable* table) { Init(name, value); table->Register(static_cast<ISVar*>(this)); }
	

	virtual void SetName(const char* name) { m_name = name; }
	virtual const char* GetName() { return m_name; }
	virtual void* GetData() { return reinterpret_cast<void*>(&m_data); }
	
	virtual void SetValue(T value)  { m_data = value; }
	T     GetValue()         { return m_data; }

	virtual const type_info& GetTypeInfo() { return typeid(T); }

	virtual ISVar* MakeCopy() { return new CSVar<T>{ m_name,m_data }; }
protected:
	const char* m_name = 0;
	T m_data;
};

//
// Type Implementation below
//

#define BEGIN_SVAR_TYPE_IMPLEMENT(type)                                                    \
template<>                                                                                 \
class CSVar<type> : public internal_SVar<type>                                             \
{                                                                                          \
public:                                                                                    \
	using VarType = type;                                                                  \
	CSVar(const char* name, type value)                    { Init(name, value); }          \
	CSVar(const char* name, type value, CSVarTable* table) { Init(name, value, table); }   \
	virtual void SetData(void* data)													   \
	{                                 													   \
		SetValue(*(VarType*)data);    													   \
	}


#define END_SVAR_TYPE_IMPLEMENT() };

#define IMPL_FORMAT_TOSTRING(strSize, format) \
virtual char* ToString()                      \
{                                             \
	char* str = new char[strSize];            \
	snprintf(str, strSize, format, m_data);   \
	return str;                               \
}                                             \


#define IMPL_FORMAT_TOSTRING_EX(strSize, format, ...) \
virtual char* ToString()                              \
{                                                     \
	char* str = new char[strSize];                    \
	snprintf(str, strSize, format, __VA_ARGS__);      \
	return str;                                       \
}                                                     \

#define IMPL_FROMSTRING(...)                          \
virtual void FromString(char* str)                    \
{                                                     \
	m_data = __VA_ARGS__;                             \
}  

//
// Type implementation
//
// Move this somewhere else?
//

BEGIN_SVAR_TYPE_IMPLEMENT(int)
	IMPL_FORMAT_TOSTRING(16, "%d")
	IMPL_FROMSTRING(atoi(str))
END_SVAR_TYPE_IMPLEMENT()

BEGIN_SVAR_TYPE_IMPLEMENT(unsigned int)
	IMPL_FORMAT_TOSTRING(16, "%u")
	IMPL_FROMSTRING(strtoul(str, nullptr, 10))
END_SVAR_TYPE_IMPLEMENT()

BEGIN_SVAR_TYPE_IMPLEMENT(long long)
	IMPL_FORMAT_TOSTRING(24, "%lld")
	IMPL_FROMSTRING(strtoll(str, nullptr, 10))
END_SVAR_TYPE_IMPLEMENT()

BEGIN_SVAR_TYPE_IMPLEMENT(unsigned long long)
	IMPL_FORMAT_TOSTRING(24, "%llu")
	IMPL_FROMSTRING(strtoull(str, nullptr, 10))
END_SVAR_TYPE_IMPLEMENT()

// Check how long floats can be at some point
BEGIN_SVAR_TYPE_IMPLEMENT(float)
	IMPL_FORMAT_TOSTRING(24, "%f")
	IMPL_FROMSTRING(strtof(str, nullptr))
END_SVAR_TYPE_IMPLEMENT()

// Check how long doubles can be at some point
BEGIN_SVAR_TYPE_IMPLEMENT(double)
	IMPL_FORMAT_TOSTRING(32, "%f")
	IMPL_FROMSTRING(strtod(str, nullptr))
END_SVAR_TYPE_IMPLEMENT()


BEGIN_SVAR_TYPE_IMPLEMENT(bool)
	virtual char* ToString()
	{
		if (m_data)
		{
			char* str = new char[5];
			const char t[5] = "true";
			memcpy(str, t, 5);
			return str;
		}
		else
		{
			char* str = new char[6];
			const char f[6] = "false";
			memcpy(str, f, 6);
			return str;
		}
	}

	IMPL_FROMSTRING(strcmp(str, "true") == 0)
END_SVAR_TYPE_IMPLEMENT()

BEGIN_SVAR_TYPE_IMPLEMENT(char*)
	
	virtual void SetValue(char* str)
	{
		FromString(str);
	}

	virtual void FromString(char* str)
	{
		// We don't know how long this string is actually going to live. Let's copy it out 
		if (!str)
		{
			// They gave us a null.. how sad.
			m_length = 0;
			m_data = nullptr;
		}

		m_length = strlen(str);

		// Out with the old
		if (m_data)
			delete[] m_data;

		// In with the new
		m_data = new char[m_length + 1];
		strncpy(m_data, str, m_length + 1);
	}

	virtual char* ToString()
	{
		char* str = new char[m_length + 1];
		strncpy(str, m_data, m_length + 1);
		return str;
	}
	
	virtual 

	size_t GetLength() { return m_length; }
private:
	size_t m_length = 0;
END_SVAR_TYPE_IMPLEMENT()
