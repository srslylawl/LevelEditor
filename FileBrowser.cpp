#include "FileBrowser.h"

#include <string>

#include "Files.h"
#include "imgui.h"
#include "ImGuiHelper.h"
#include "Resources.h"
#include "Strings.h"
#include "Texture.h"
#include "TextureSheet.h"
#include "Tile.h"


FileBrowser::FileBrowser(const char* start_directory, std::string title, std::function<void(FileBrowserFile)> onFileClick,
						 bool isTileSheetBrowser, std::function<bool(FileBrowserFile)> shouldHighlight, std::function<void(FileBrowserFile&)> onFileEdit) :
	name(std::move(title)),
	fileBrowserID(++currentID),
	isTileSheetBrowser(isTileSheetBrowser),
	onFileClick(std::move(onFileClick)),
	shouldHighlight(std::move(shouldHighlight)),
	onFileEdit(std::move(onFileEdit)) {

	const auto path = Files::GetAbsolutePath(start_directory);
	currentDirectory = path;

	Resources::TryGetInternalTexture(Strings::Folder, folderTexture);
	Resources::TryGetInternalTexture(Strings::Previous_Directory, returnTexture);

	RefreshCurrentDirectory();
}

bool DrawFileButton(const Rendering::Texture* texture, const int elementCount, const std::string& name, const std::string& description, const int size, bool shouldHighlight = false, const FileBrowserFile* file = nullptr, bool* out_rightClicked = nullptr) {
	using namespace ImGui;

	ImVec2 startPos = GetCursorStartPos();

	const int iconSideLength = size;
	const int totalItemWidth = iconSideLength + 3;
	startPos.x += (startPos.x + totalItemWidth) * elementCount;

	ImVec2 IconDrawPos(startPos.x, startPos.y);


	SetCursorPos(IconDrawPos);
	PushID(name.c_str());
	if (shouldHighlight) ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(255, 255, 255));
	const bool clicked = ImageButton(reinterpret_cast<void*>(texture->GetTextureID()), ImVec2(iconSideLength, iconSideLength), ImVec2(0, 1), ImVec2(1, 0));
	const bool wasRightClicked = IsItemClicked(ImGuiMouseButton_Right);

	if(wasRightClicked && out_rightClicked != nullptr) {
		*out_rightClicked = true;
	}

	if (shouldHighlight) PopStyleColor();

	const bool allowDragAndDrop = file != nullptr &&
		(file->FileType == FileBrowserFileType::Sprite || file->FileType == FileBrowserFileType::TextureSheet);
	if (allowDragAndDrop) {
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
			std::string payloadType;
			void* payload = nullptr;
			switch (file->FileType) {
				case FileBrowserFileType::Sprite:
					payloadType = "Texture";
					payload = (void*)texture;
					break;
				case FileBrowserFileType::TextureSheet:
					payloadType = "TextureSheet";
					payload = file->Data;
					break;
				default:
					throw std::exception("unexpected file type for drag and drop", static_cast<int>(file->FileType));
			}
			ImGui::SetDragDropPayload(payloadType.c_str(), &payload, sizeof(void*));
			ImGuiHelper::Image(texture->GetTextureID());
			Text(file->directory_entry.path().filename().string().c_str());
			ImGui::EndDragDropSource();
		}
	}


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
	constexpr ImGuiWindowFlags fileBrowserFlags = ImGuiWindowFlags_AlwaysAutoResize;
	std::string imguiID = "FileBrowser_" + std::to_string(fileBrowserID);
	std::string windowTitle = Files::GetRelativePath(currentDirectory.string()) + "###" + imguiID; /// '###' allows manipulating of window title

	int buttonSize = 32;

	PushID(imguiID.c_str());
	if (Begin(windowTitle.c_str(), nullptr, fileBrowserFlags)) {
		int buttonIndex = 0;
		if (!subDirectoryStack.empty()) {
			//not in base directory, display "up" button
			const std::string returnButton = imguiID + "ReturnButton";
			if (DrawFileButton(returnTexture, buttonIndex, returnButton, "Return to previous directory.", buttonSize)) {
				ReturnToPreviousDirectory();
			}
			++buttonIndex;
		}

		//display subdirectories first
		for (auto& subDir : currentSubFolders) {
			const std::string currButton = imguiID + "_Subdirectories_" + std::to_string(buttonIndex);
			if (DrawFileButton(folderTexture, buttonIndex, currButton, subDir.path().filename().string(), buttonSize)) {
				ChangeDirectory(subDir.path());
			}

			++buttonIndex;
		}

		//display files
		for (auto& file : currentItems) {
			const std::string currButton = imguiID + "_Items_" + std::to_string(buttonIndex);
			const bool highlight = shouldHighlight == nullptr ? false : shouldHighlight(file);
			const bool isSupportedFile = file.FileType != FileBrowserFileType::Unsupported;
			bool wasRightClicked = false;
			if (DrawFileButton(file.Texture, buttonIndex, currButton, file.directory_entry.path().filename().string(), buttonSize, highlight, &file, &wasRightClicked)) {
				if (onFileClick != nullptr)
					onFileClick(file);
			}
			if(wasRightClicked) {
				if(onFileEdit != nullptr)
					onFileEdit(file);
			}
			++buttonIndex;
		}
		//resize window with title
		SetCursorPosX(CalcTextSize(windowTitle.c_str()).x);
	}
	End();
	PopID();
}

void FileBrowser::RefreshCurrentTileSheetDirectory() {
	for (auto& dirEntry : Files::GetDirectoryIterator(currentDirectory.string().c_str())) {
		if (dirEntry.is_directory()) {
			currentSubFolders.push_back(dirEntry);
			continue;
		}

		if (!dirEntry.exists() || !dirEntry.is_regular_file())
			continue;

		FileBrowserFile fileBrowserFile(dirEntry);
		fileBrowserFile.Texture = Rendering::Texture::Empty();

		const bool isTextureSheetFile = dirEntry.path().has_extension() && dirEntry.path().extension().string() == TextureSheet::FileEnding;
		if (!isTextureSheetFile) continue;

		auto relativePath = Files::GetRelativePath(dirEntry.path().string());
		TextureSheet* textureSheet;
		if (!Resources::TryGetTextureSheet(relativePath.c_str(), textureSheet)) {
			std::cout << "ERROR: Unable to get textureSheet: " << relativePath.c_str() << std::endl;
			continue;
		}
		if (textureSheet->GetMainTexture() != nullptr) {
			fileBrowserFile.Texture = textureSheet->GetMainTexture();
		}

		fileBrowserFile.Data = textureSheet;
		fileBrowserFile.FileType = FileBrowserFileType::TextureSheet;

		currentItems.emplace_back(fileBrowserFile);
	}
}

void FileBrowser::RefreshCurrentGenericDirectory() {
	for (auto& dirEntry : Files::GetDirectoryIterator(currentDirectory.string().c_str())) {
		if (dirEntry.is_directory()) {
			currentSubFolders.push_back(dirEntry);
			continue;
		}

		if (dirEntry.exists() && dirEntry.is_regular_file()) {
			FileBrowserFile fileBrowserFile(dirEntry);
			fileBrowserFile.Texture = Rendering::Texture::Empty();

			if (Files::IsSupportedImageFormat(dirEntry.path().string().c_str())) {
				//Is Image
				auto relativePath = Files::GetRelativePath(dirEntry.path().string());
				Resources::LoadTexture(relativePath.c_str(), fileBrowserFile.Texture, true);

				fileBrowserFile.Data = fileBrowserFile.Texture;
				fileBrowserFile.FileType = FileBrowserFileType::Sprite;
			}
			else if (dirEntry.path().has_extension() && dirEntry.path().extension().string() == Tiles::Tile::FileEnding) {
				//Is Tile
				auto relativePath = Files::GetRelativePath(dirEntry.path().string());
				Tiles::Tile* tile;
				if (!Resources::TryGetTile(relativePath.c_str(), tile, true)) {
					std::cout << "ERROR: Unable to get tile: " << relativePath.c_str() << std::endl;
					continue;
				}
				if (!Resources::TryGetTexture(tile->DisplayTexture.c_str(), fileBrowserFile.Texture)) {
					std::cout << "ERROR: Unable to get texture for tile: " << relativePath.c_str() << " texture: " << tile->DisplayTexture.c_str() << std::endl;
				}
				fileBrowserFile.Data = tile;
				fileBrowserFile.FileType = FileBrowserFileType::Tile;

			}
			else {
				Resources::TryGetInternalTexture(Strings::Unknown_File, fileBrowserFile.Texture);
				fileBrowserFile.FileType = FileBrowserFileType::Unsupported;
			}
			currentItems.emplace_back(fileBrowserFile);
		}
	}
}

void FileBrowser::RefreshCurrentDirectory() {
	currentItems.clear();
	currentSubFolders.clear();

	if (isTileSheetBrowser) {
		RefreshCurrentTileSheetDirectory();
	}
	else {
		RefreshCurrentGenericDirectory();
	}

	//sort directories by name
	std::sort(currentSubFolders.begin(), currentSubFolders.end(), [](const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b) -> bool {
		return a.path().filename().string() < b.path().filename().string();
	});
}

void FileBrowser::ChangeDirectory(const std::filesystem::path& new_directory) {
	subDirectoryStack.push(currentDirectory);
	currentDirectory = new_directory;
	RefreshCurrentDirectory();
}

void FileBrowser::ReturnToPreviousDirectory() {
	currentDirectory = subDirectoryStack.top();
	subDirectoryStack.pop();
	RefreshCurrentDirectory();
}
