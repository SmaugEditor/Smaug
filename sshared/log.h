#pragma once
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <vector>
#include <typeinfo>
#include <cstring>

#ifdef _DEBUG
#define SASSERT(cond) Log::Assert(cond, #cond, __LINE__, __FILE__, __FUNCTION__)
#define SASSERT_S(cond) Log::AssertSilent(cond, #cond, __LINE__, __FILE__, __FUNCTION__)
#else
// If you're using SASSERT, NEVER EVER put something like i++ in the arguments
// DON'T DO IT
#define SASSERT(cond)
#define SASSERT_S(cond)
#endif

#define SASSERT_FATAL(cond) Log::AssertFatal(cond, #cond, __LINE__, __FILE__, __FUNCTION__)

namespace Log
{


enum class MessageType
{
	NONE,

	DEBUG,
	MESSAGE,
	WARNING,
	// Thank you msvc, for preventing me from using ERROR
	FAULT,
	FATAL,
};

// Used by the logger for temp working space. Don't use this if you're not the logger!
char* WorkingBuf(size_t len);

// On failure, logs and pops up a message box
bool Assert(bool condition, const char* expression, int line, const char* file, const char* function);

// On failure, logs only
bool AssertSilent(bool condition, const char* expression, int line, const char* file, const char* function);

// On failure, log, popup, and exit
bool AssertFatal(bool condition, const char* expression, int line, const char* file, const char* function);

// Sets where we drain all of our logs into
typedef void(*messageSink_t)(MessageType, const char*);
void SetSink(messageSink_t sink);

// Pours the message right into the sink
void Drain(MessageType type, char* str);

// Formats and drains our message
template <typename... T>
void DrainFormat(MessageType type, const char* str, T... args)
{
	// How many bytes do we need?
	int size = snprintf(nullptr, 0, str, args...) + 1;

	// I would use std::format, if it was supported
	// Maybe make use of it at a later date!

	char* buf = WorkingBuf(size);

	snprintf(buf, size, str, args...);
	Drain(type, buf);
}

// Quality of life wrappers around DrainFormat
// Debug, Msg, Error, Warn, Fatal

// Won't log while in release
template <typename... T>
void Debug(const char* str, T... args)
{
#ifdef _DEBUG
	DrainFormat(MessageType::DEBUG, str, args...);
#endif
}


template <typename... T>
void Print(const char* str, T... args)
{
	DrainFormat(MessageType::NONE, str, args...);
}


template <typename... T>
void Msg(const char* str, T... args)
{
	DrainFormat(MessageType::MESSAGE, str, args...);
}

// Will throw a pop up
template <typename... T>
void Fault(const char* str, T... args)
{
	DrainFormat(MessageType::FAULT, str, args...);
}

// Will write yellow text to the console
template <typename... T>
void Warn(const char* str, T... args)
{
	DrainFormat(MessageType::WARNING, str, args...);
}

// Will throw a fatal popup
template <typename... T>
void Fatal(const char* str, T... args)
{
	DrainFormat(MessageType::FATAL, str, args...);
}

// "This Tagged" Messages
// - Creates logs that are prefixed with "this" class
// - Relies on the calling class being prefixed with a 'C'

template<typename T>
constexpr const char* ClassName()
{
	// Our classes always start with a C, which means we can use it to poke around and find the start of the class, even this is implementation defined function
	const char* name = typeid(T).name();
	const char* pos = strchr(name, 'C');

	// Found the C? Increment by one, to skip the C, and return it 
	if (pos)
		return pos + 1;

	// Failed to find it! Return the mangled name???
	return name;
}

#define TDebug(str, ...) Debug("[%s] " str, Log::ClassName<decltype(*this)>()	__VA_OPT__(,) __VA_ARGS__)
#define TPrint(str, ...) Print("[%s] " str, Log::ClassName<decltype(*this)>()	__VA_OPT__(,) __VA_ARGS__)
#define TMsg(str, ...) Msg("[%s] " str, Log::ClassName<decltype(*this)>()		__VA_OPT__(,) __VA_ARGS__)
#define TFault(str, ...) Fault("[%s] " str, Log::ClassName<decltype(*this)>()	__VA_OPT__(,) __VA_ARGS__)
#define TWarn(str, ...) Warn("[%s] " str, Log::ClassName<decltype(*this)>()		__VA_OPT__(,) __VA_ARGS__)
#define TFatal(str, ...) Fatal("[%s] " str, Log::ClassName<decltype(*this)>()	__VA_OPT__(,) __VA_ARGS__)
}
