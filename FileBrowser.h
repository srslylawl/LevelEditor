#pragma once
#include <filesystem>
#include <stack>

#include "Files.h"

class FileBrowser {
	std::string name;
	std::filesystem::path currentDirectory;

	std::stack<std::filesystem::path> directoryStack;

	int fileBrowserID;
	inline static int currentID = 0;
public:
	FileBrowser(const char* start_directory, std::string title) : name(std::move(title)), fileBrowserID(++currentID) {
		const auto path = Files::GetAbsolutePath(start_directory);
		currentDirectory = path;

	}

	void DearImGuiWindow();
};

