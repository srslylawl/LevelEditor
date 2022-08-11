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
						 std::function<bool(FileBrowserFile)> shouldHighlight,
						 std::function<void(FileBrowserFile&)> onFileEdit,
						 std::function<void(FileBrowser*)> onNewFile) :
	name(std::move(title)),
	fileBrowserID(++currentID),
	onFileClick(std::move(onFileClick)),
	shouldHighlight(std::move(shouldHighlight)),
	onFileEdit(std::move(onFileEdit)),
	onNewFile(onNewFile) {

	const auto path = Files::GetAbsolutePath(start_directory);
	currentDirectory = path;

	Resources::TryGetInternalTexture(Strings::Icon_Folder, folderTexture);
	Resources::TryGetInternalTexture(Strings::Icon_Previous_Directory, returnTexture);
	Resources::TryGetInternalTexture(Strings::Icon_New_File, newFileTexture);

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

	if (wasRightClicked && out_rightClicked != nullptr) {
		*out_rightClicked = true;
	}

	if (shouldHighlight) PopStyleColor();

	const bool allowDragAndDrop = file != nullptr &&
		(file->AssetHeader.aType == AssetType::Texture || file->AssetHeader.aType == AssetType::TextureSheet);
	if (allowDragAndDrop) {
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
			std::string payloadType;
			void* payload = nullptr;
			switch (file->AssetHeader.aType) {
				case AssetType::Texture:
					payloadType = "Texture";
					payload = (void*)texture;
					break;
				case AssetType::TextureSheet:
					payloadType = "TextureSheet";
					payload = file->Data;
					break;
				default:
					throw std::exception("unexpected file type for drag and drop", static_cast<int>(file->AssetHeader.aType));
			}
			ImGui::SetDragDropPayload(payloadType.c_str(), &payload, sizeof(void*));
			ImGuiHelper::Image(texture->GetTextureID());
			TextUnformatted(file->AssetHeader.aPath.filename().string().c_str());
			ImGui::EndDragDropSource();
		}
	}


	PopID();
	if (IsItemHovered()) {
		BeginTooltip();
		TextUnformatted(description.c_str());
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
				break;
			}

			++buttonIndex;
		}

		//display files
		for (auto& file : currentItems) {
			const std::string currButton = imguiID + "_Items_" + std::to_string(buttonIndex);
			const bool highlight = shouldHighlight == nullptr ? false : shouldHighlight(file);
			bool wasRightClicked = false;
			if (DrawFileButton(file.Texture, buttonIndex, currButton, file.AssetHeader.aPath.filename().string(), buttonSize, highlight, &file, &wasRightClicked)) {
				if (onFileClick != nullptr)
					onFileClick(file);
			}
			if (wasRightClicked) {
				if (onFileEdit != nullptr)
					onFileEdit(file);
			}
			++buttonIndex;
		}

		if (onNewFile != nullptr) {
			//Show a "create new file" button
			if (DrawFileButton(newFileTexture, buttonIndex++, "Create new...", "Click to create a new object.", buttonSize)) {
				onNewFile(this);
			}
		}
		//resize window with title
		SetCursorPosX(CalcTextSize(windowTitle.c_str()).x);
	}
	End();
	PopID();
}

void FileBrowser::RefreshCurrentDirectory() {
	currentItems.clear();
	currentSubFolders.clear();

	std::vector<AssetHeader> headers;
	Resources::LoadDirectory(currentDirectory.string().c_str(), false, false, &headers);
	for (auto& dirEntry : Files::GetDirectoryIterator(currentDirectory.string().c_str())) {
		if (dirEntry.is_directory()) {
			currentSubFolders.push_back(dirEntry);
		}
	}

	for (const auto& header : headers) {
		FileBrowserFile fileBrowserFile(header, this);
		fileBrowserFile.Texture = Rendering::Texture::Empty();

		switch (header.aType) {
			case AssetType::Texture:
			{
				if (Resources::TryGetTexture(header.aId, fileBrowserFile.Texture)) {
					fileBrowserFile.Data = fileBrowserFile.Texture;
				}
				break;
			}
			case AssetType::Tile:
			{
				Tiles::Tile* t = nullptr;
				if (!Resources::TryGetTile(header.aId, t)) {
					std::cout << "ERROR: Unable to get tile: " << header.aPath.c_str() << std::endl;
					continue;
				}
				fileBrowserFile.Data = t;
				if (!Resources::TryGetTexture(t->DisplayTexture, fileBrowserFile.Texture)) {
					std::cout << "ERROR: Unable to get texture for tile: " << header.aPath.c_str() << " texture: " << t->DisplayTexture.ToString() << std::endl;
				}
				break;
			}
			case AssetType::TextureSheet:
			{
				TextureSheet* textureSheet;
				if (!Resources::TryGetTextureSheet(header.aId, textureSheet)) {
					std::cout << "ERROR: Unable to get textureSheet: " << header.aPath.c_str() << std::endl;
					continue;
				}
				if (textureSheet->GetMainTexture() != nullptr) {
					fileBrowserFile.Texture = textureSheet->GetMainTexture();
				}
				fileBrowserFile.Data = textureSheet;
				break;
			}
			case AssetType::Level: break;
			case AssetType::UNKNOWN: break;
			case AssetType::TextureInternal: break;
			default: break;
		}
		currentItems.emplace_back(fileBrowserFile);
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
