#pragma once
#include <filesystem>
#include <functional>
#include <stack>

#include "FileBrowserFile.h"

namespace Rendering {
	class Texture;
}

class FileBrowser {
	std::string name;
	std::filesystem::path currentDirectory;

	std::vector<FileBrowserFile> currentItems;
	std::vector<std::filesystem::directory_entry> currentSubFolders;

	std::stack<std::filesystem::path> subDirectoryStack;

	int fileBrowserID;
	inline static int currentID = 0;

	Rendering::Texture* folderTexture = nullptr;
	Rendering::Texture* returnTexture = nullptr;
	Rendering::Texture* newFileTexture = nullptr;

	std::function<void(FileBrowserFile)> onFileClick;
	std::function<bool(FileBrowserFile)> shouldHighlight;
	std::function<void(FileBrowserFile&)> onFileEdit;
	std::function<void(FileBrowser*)> onNewFile;
public:
	FileBrowser(const char* start_directory, std::string title, std::function<void(FileBrowserFile)> onFileClick = nullptr,
	            std::function<bool(FileBrowserFile)> shouldHighlight = nullptr, std::function<void(FileBrowserFile&)> onFileEdit = nullptr,
	            std::function<void(FileBrowser*)> onNewFile = nullptr);
	void RenderRearImGuiWindow();
	void RefreshCurrentDirectory();
	void ChangeDirectory(const std::filesystem::path& new_directory);
	void ReturnToPreviousDirectory();
};

