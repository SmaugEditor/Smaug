#include "filesystem.h"
#include <portable-file-dialogs/portable-file-dialogs.h>
#include <fstream>


void RunningDirectory(char* path, size_t length)
{
#ifdef _WIN32
    GetCurrentDirectory(length, path);
#else
#error UNSUPPORTED_PLATFORM
#endif

}

bool filesystem::SaveFile(const char* path, char* data)
{
    std::ofstream file;
    file.open(path);
    file << data;
    file.close();
    return false;
}

bool filesystem::SaveFileWithDialog(char* data, const char* fileType)
{

    char runningDir[128];
    RunningDirectory(runningDir, 128);

    pfd::save_file f = pfd::save_file("Choose files to read", runningDir,
        { fileType, fileType,
          "All Files", "*" });
    
    SaveFile(f.result().c_str(), data);

    return false;
}
