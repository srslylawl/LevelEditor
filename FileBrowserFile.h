#pragma once
#include "Texture.h"

class FileBrowser;


class FileBrowserFile {
public:
	void* Data = nullptr;
	AssetHeader AssetHeader;

	FileBrowserFile(::AssetHeader header, FileBrowser* fileBrowser) : AssetHeader(std::move(header)), FileBrowser(fileBrowser) { }

	Rendering::Texture* Texture = nullptr;
	FileBrowser* FileBrowser = nullptr;
};

