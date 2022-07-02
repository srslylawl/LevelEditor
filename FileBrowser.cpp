#include "FileBrowser.h"

#include <iostream>
#include <string>

#include "Files.h"
#include "imgui.h"
#include "Resources.h"
#include "Strings.h"
#include "Texture.h"
#include "Tile.h"

FileBrowser::FileBrowser(const char* start_directory, std::string title, std::function<void(std::string)> onFileClick) : name(std::move(title)), fileBrowserID(++currentID), onFileClick(std::move(onFileClick)) {
	const auto path = Files::GetAbsolutePath(start_directory);
	currentDirectory = path;

	if (Rendering::Texture* subDirectoryTexture; Resources::TryGetInternalTexture(Strings::Folder, subDirectoryTexture)) {
		folderTextureId = subDirectoryTexture->GetTextureID();
	}
	RefreshCurrentDirectory();
}

bool DrawFileButton(const int textureID, const int elementCount, const std::string& name, const std::string& description, const int size) {
	using namespace ImGui;
	ImVec2 startPos = GetCursorStartPos();

	const int iconSideLength = size;
	const int totalItemWidth = iconSideLength + 3;
	startPos.x += (startPos.x + totalItemWidth) * elementCount;

	ImVec2 IconDrawPos(startPos.x, startPos.y);

	SetCursorPos(IconDrawPos);
	PushID(name.c_str());
	const bool clicked = ImageButton((void*)textureID, ImVec2(iconSideLength, iconSideLength), ImVec2(0, 1), ImVec2(1, 0));
	PopID();
	if (IsItemHovered()) {
		BeginTooltip();
		Text(description.c_str());
		EndTooltip();
	}
	return clicked;
}

void FileBrowser::RenderRearImGuiWindow() {
	using namespace ImGui;
	constexpr ImGuiWindowFlags fileBrowserFlags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_MenuBar;
	static bool fileBrowserOpen = true;
	std::string imguiID = "FileBrowser_" + std::to_string(fileBrowserID);

	int buttonSize = 32;

	PushID(imguiID.c_str());
	if (Begin(name.c_str(), &fileBrowserOpen, fileBrowserFlags)) {
		BeginMenuBar();
		Text(Files::GetRelativePath(currentDirectory.string()).c_str());
		EndMenuBar();


		int buttonIndex = 0;

		if (!subDirectoryStack.empty()) {
			//not in base directory, display "up" button
			const std::string returnButton = imguiID + "ReturnButton";
			if (DrawFileButton(returnTextureId, buttonIndex, returnButton, "Return to previous directory.", buttonSize)) {
				ReturnToPreviousDirectory();
			}
			++buttonIndex;
		}

		//display subdirectories first
		for (const auto& subDir : currentSubFolders) {
			const std::string currButton = imguiID + "_Subdirectories_" + std::to_string(buttonIndex);
			if (DrawFileButton(folderTextureId, buttonIndex, currButton, subDir.path().filename().string(), buttonSize)) {
				ChangeDirectory(subDir.path());
			}

			++buttonIndex;
		}

		//display files
		for (const auto& [entry, textureID] : currentItems) {
			const std::string currButton = imguiID + "_Items_" + std::to_string(buttonIndex);
			if (DrawFileButton(textureID, buttonIndex, currButton, entry.path().filename().string(), buttonSize)) {
				if (onFileClick != nullptr)
				onFileClick(Files::GetRelativePath(entry.path()));
			}
			++buttonIndex;
		}

	}
	End();
	PopID();
}

void FileBrowser::RefreshCurrentDirectory() {
	currentItems.clear();
	currentSubFolders.clear();

	for (auto& dirEntry : Files::GetDirectoryIterator(currentDirectory.string().c_str())) {
		if (dirEntry.is_directory()) {
			currentSubFolders.push_back(dirEntry);
			continue;
		}

		if (dirEntry.exists() && dirEntry.is_regular_file()) {
			Rendering::Texture* texture = nullptr;
			int textureID = 0;
			if (Files::IsSupportedImageFormat(dirEntry.path().string().c_str())) {
				//Is Image
				auto relativePath = Files::GetRelativePath(dirEntry.path().string());
				Resources::TryGetTexture(relativePath.c_str(), texture);
			}
			else if (dirEntry.path().has_extension() && dirEntry.path().extension().string() == Tiles::Tile::fileEnding) {
				//Is Tile
				auto relativePath = Files::GetRelativePath(dirEntry.path().string());
				if(Tiles::Tile* tile; Resources::TryGetTile(relativePath.c_str(), tile)) {
					Resources::TryGetTexture(tile->Texture.c_str(), texture);
				}
			}
			else {
				Resources::TryGetInternalTexture(Strings::Unknown_File, texture);
			}

			if (texture != nullptr) textureID = texture->GetTextureID();
			currentItems.emplace_back(dirEntry, textureID);
		}
	}

	//sort directories by name
	std::sort(currentSubFolders.begin(), currentSubFolders.end(), [](const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b) -> bool
		{
			return a.path().filename().string() < b.path().filename().string();
		});
}

void FileBrowser::ChangeDirectory(std::filesystem::path new_directory) {
	subDirectoryStack.push(currentDirectory);
	currentDirectory = new_directory;
	RefreshCurrentDirectory();
}

void FileBrowser::ReturnToPreviousDirectory() {
	currentDirectory = subDirectoryStack.top();
	subDirectoryStack.pop();
	RefreshCurrentDirectory();
}
