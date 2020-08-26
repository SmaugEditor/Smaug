#pragma once
namespace filesystem
{
	bool SaveFile(const char* path, char* data);
	bool SaveFileWithDialog(char* data, const char* fileType = "*.txt");
};

