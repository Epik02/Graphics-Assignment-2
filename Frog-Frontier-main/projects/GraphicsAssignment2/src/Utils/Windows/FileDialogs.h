#pragma once
#include <optional>
#include <string>

class FileDialogs
{
public:
	static std::optional<std::string> OpenFile(const char* filter = "All Files\0*.*\0\0");
	static std::optional<std::string> SaveFile(const char* filter = "All Files\0*.*\0\0");
	static std::optional<std::string> SelectFolder(const char* filter = "Folders\0*\0\0");
};