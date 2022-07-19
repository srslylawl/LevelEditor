#pragma once
#include <filesystem>
#include "Texture.h"

class FileBrowser;

enum class FileBrowserFileType {
	Unsupported = 1 << 0,
	Sprite = 1 << 1,
	Tile = 1 << 2,
	TextureSheet = 1 << 3
};
class FileBrowserFile {
public:
	void* Data = nullptr;
	std::filesystem::directory_entry directory_entry;

	FileBrowserFile(std::filesystem::directory_entry dirEntry, FileBrowser* fileBrowser) : directory_entry(dirEntry), FileType(FileBrowserFileType::Unsupported), FileBrowser(fileBrowser) { }
	FileBrowserFile(void* data, const FileBrowserFileType type) : Data(data), FileType(type) { }

	FileBrowserFileType FileType;
	Rendering::Texture* Texture = nullptr;
	FileBrowser* FileBrowser = nullptr;
};

