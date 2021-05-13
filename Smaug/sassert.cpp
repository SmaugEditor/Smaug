#include "sassert.h"
#include <cstdio>
#ifdef _WIN32
#include <windows.h>
#endif


// Cheesy move to strip off the path from the file name 
consteval int findInFILE(char s)
{
    constexpr const char path[] = __FILE__;
    constexpr const int len = sizeof(path);
    for (int i = len - 1; i != 0; i--)
        if (path[i] == s)
            return i;
    return -1;
}

consteval int baseFilePathLength()
{
    int slash;
#ifdef _WIN32
    slash = findInFILE('\\');
    if(slash < 0)
        slash = findInFILE('/');
#else
    slash = findInFILE('/');
    if (slash < 0)
        slash = findInFILE('\\');
#endif
    if (slash > 0)
        return slash + 1;
    return 0;
}

static constexpr int s_baseFilePathLength = baseFilePathLength();


// Use this so that all assertions look consistent
void formatAssertion(char* message, size_t length, const char* expression, int line, const char* file, const char* function)
{
    snprintf(message, 1024, "-= Assertion failed! =-\n%s\n%s in %s: Line %d\n\n", expression, function, file + s_baseFilePathLength, line);
}

bool smaugAssert(bool condition, const char* expression, int line, const char* file, const char* function)
{
    if (!condition)
    {
        char message[1024];
        formatAssertion(message, 1024, expression, line, file, function );

#ifdef _WIN32
        int ret = MessageBoxA(nullptr, message, "Assertion failed!", MB_ABORTRETRYIGNORE);

        // This is counterintuitive to the buttons...
       
        printf(message);
        switch (ret)
        {
        case IDABORT:
            exit(1);
            break;
        case IDRETRY:
            DebugBreak();
            break;
        
        }
#else
    if(!condition)
        printf("Assertion failed (%s:%d) %s: '%s'", file, line, function, expression);
#endif
    }
    return condition;
}


bool smaugAssertSilent(bool condition, const char* expression, int line, const char* file, const char* function)
{
    if (!condition)
    {
        char message[1024];
        formatAssertion(message, 1024, expression, line, file, function);
        printf(message);
    }
    return condition;
}
