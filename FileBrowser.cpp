#include "FileBrowser.h"

#include <iostream>
#include <string>

#include "Files.h"
#include "imgui.h"
#include "Resources.h"
#include "Strings.h"
#include "Texture.h"

void FileBrowser::DearImGuiWindow() {
	using namespace ImGui;
	// File Browser
	constexpr ImGuiWindowFlags fileBrowserFlags = ImGuiWindowFlags_AlwaysAutoResize;
	static bool fileBrowserOpen = true;
	std::string imguiID = "FileBrowser_" + std::to_string(fileBrowserID);
	PushID(imguiID.c_str());
	if (Begin(name.c_str(), &fileBrowserOpen, fileBrowserFlags)) {
		if (!directoryStack.empty()) {
			//not in base directory, display "up" button
			std::string returnButton = imguiID + "ReturnButton";
			PushID(returnButton.c_str());
			if (Button("Return", ImVec2(32, 32))) {
				currentDirectory = directoryStack.top();
				directoryStack.pop();
			}
			PopID();
			SameLine();
		}

		auto dirIterator = Files::GetDirectoryIterator(currentDirectory.string().c_str());
		int buttonIndex = 0;
		for (; dirIterator != end(dirIterator); ++dirIterator) {
			std::string currButton = imguiID + "FileButtons_" + std::to_string(buttonIndex);
			PushID(currButton.c_str());
			bool isDirectory = dirIterator->is_directory();
			Rendering::Texture* tex;
			int textureID = 0;
			if (isDirectory) {
				if (Resources::TryGetInternalTexture(Strings::Folder, tex)) {
					textureID = tex->GetTextureID();
				}
				if (ImageButton((void*)textureID, ImVec2(32, 32), ImVec2(0, 1), ImVec2(1, 0))) {
					directoryStack.push(currentDirectory);
					currentDirectory = dirIterator->path();
				}
			}
			else {
				if (Files::IsSupportedImageFormat(dirIterator->path().string().c_str())) {
					auto relativePath = Files::GetRelativePath(dirIterator->path().string());
					if (Resources::TryGetTexture(relativePath.c_str(), tex)) {
						textureID = tex->GetTextureID();
					}
				}
				else {
					if (Resources::TryGetInternalTexture(Strings::Unknown_File, tex)) {
						textureID = tex->GetTextureID();
					}
				}

				if (ImageButton((void*)textureID, ImVec2(32, 32), ImVec2(0, 1), ImVec2(1, 0))) {
					std::cout << "Clicked " << dirIterator->path() << std::endl;
				}
			}
			PopID();
			++buttonIndex;
			SameLine();
		}
	}
	End();
	PopID();
}
