#include "FileBrowser.h"

#include <string>

#include "Files.h"
#include "imgui.h"
#include "ImGuiHelper.h"
#include "Resources.h"
#include "Strings.h"
#include "Texture.h"
#include "Tile.h"

FileBrowser::FileBrowser(const char* start_directory, std::string title, std::function<void(FileBrowserFile)> onFileClick,
	std::function<bool(FileBrowserFile)> shouldHighlight) :
	name(std::move(title)),
	fileBrowserID(++currentID),
	onFileClick(std::move(onFileClick)),
	shouldHighlight(std::move(shouldHighlight)) {

	const auto path = Files::GetAbsolutePath(start_directory);
	currentDirectory = path;

	Resources::TryGetInternalTexture(Strings::Folder, folderTexture);
	Resources::TryGetInternalTexture(Strings::Previous_Directory, returnTexture);

	RefreshCurrentDirectory();
}

bool DrawFileButton(const Rendering::Texture* texture, const int elementCount, const std::string& name, const std::string& description, const int size, bool shouldHighlight = false, bool allowDragAndDrop = false) {
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
	if (shouldHighlight) PopStyleColor();
	if (allowDragAndDrop) {
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
			ImGui::SetDragDropPayload("Texture", &texture, sizeof(Rendering::Texture**));
			ImGuiHelper::Image(texture->GetTextureID());
			Text(texture->GetFileName().c_str());
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
		for (const auto& file : currentItems) {
			const std::string currButton = imguiID + "_Items_" + std::to_string(buttonIndex);
			const bool highlight = shouldHighlight == nullptr ? false : shouldHighlight(file);
			const bool isSupportedFile = file.FileType != FileBrowserFileType::Unsupported;
			if (DrawFileButton(file.Texture, buttonIndex, currButton, file.directory_entry.path().filename().string(), buttonSize, highlight, isSupportedFile)) {
				if (onFileClick != nullptr)
					onFileClick(file);
			}
			++buttonIndex;
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
			else if (dirEntry.path().has_extension() && dirEntry.path().extension().string() == Tiles::Tile::fileEnding) {
				//Is Tile
				auto relativePath = Files::GetRelativePath(dirEntry.path().string());
				Tiles::Tile* tile;
				if (!Resources::TryGetTile(relativePath.c_str(), tile)) {
					std::cout << "ERROR: Unable to get tile: " << relativePath.c_str() << std::endl;
				}
				Resources::TryGetTexture(tile->Texture.c_str(), fileBrowserFile.Texture);
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

	//sort directories by name
	std::sort(currentSubFolders.begin(), currentSubFolders.end(), [](const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b) -> bool
		{
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
