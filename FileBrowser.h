#pragma once
#include <filesystem>
#include <functional>
#include <stack>

class FileBrowser {
	std::string name;
	std::filesystem::path currentDirectory;

	std::vector<std::pair<std::filesystem::directory_entry, int>> currentItems;
	std::vector<std::filesystem::directory_entry> currentSubFolders;

	std::stack<std::filesystem::path> subDirectoryStack;

	int fileBrowserID;
	inline static int currentID = 0;

	int folderTextureId = 0;
	int returnTextureId = 0;

	std::function<void(std::string)> onFileClick;
public:
	FileBrowser(const char* start_directory, std::string title, std::function<void(std::string)> onFileClick);

	void RenderRearImGuiWindow();
	void RefreshCurrentDirectory();
	void ChangeDirectory(std::filesystem::path new_directory);
	void ReturnToPreviousDirectory();
};

