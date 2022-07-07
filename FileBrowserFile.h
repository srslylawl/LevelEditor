#pragma once
#include <filesystem>

#include "Texture.h"

enum class FileBrowserFileType {
	Unsupported = 1 << 0,
	Sprite = 1 << 1,
	Tile = 1 << 2
};
class FileBrowserFile {
public:
	void* Data;
	std::filesystem::directory_entry directory_entry;


	FileBrowserFile(std::filesystem::directory_entry dirEntry) : directory_entry(dirEntry), FileType(FileBrowserFileType::Unsupported) { }
	FileBrowserFile(void* data, const FileBrowserFileType type) : Data(data), FileType(type) { }

	FileBrowserFileType FileType;
	Rendering::Texture* Texture = nullptr;
};

