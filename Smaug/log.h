#pragma once
#include <stdio.h>
#include <stdarg.h>
#include <malloc.h>


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

	// Try to not kill our stack please?
	SASSERT_FATAL(size < 4096);

	// I would use std::format, if it was supported
	// Maybe make use of it at a later date!
	char* buf = (char*)alloca(size);
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

}
