#include "sassert.h"
#include <cstdio>
#ifdef _WIN32
#include <windows.h>
#else
#error UNIMPLEMENTED PLATFORM
#endif

bool smaugAssert(bool condition, const char* expression, int line, const char* file, const char* function)
{
    if (!condition)
    {
        char message[1024];
        snprintf(message, 1024, "Assertion failed!\n%s\n%s\n%s: Line %d", expression, function, file, line);
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
#error UNIMPLEMENTED PLATFORM
#endif
    }
    return condition;
}
