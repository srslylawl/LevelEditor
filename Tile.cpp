#include "Tile.h"

#include <fstream>
#include <iostream>

#include "Files.h"
#include "imgui.h"
#include "ImGuiHelper.h"
#include "Resources.h"
#include "Serialization.h"
#include "Texture.h"
#include "TileMap.h"

namespace Tiles {
	bool Tile::Deserialize(std::istream& iStream, Tile*& out_tile) {
		out_tile = new Tile();

		out_tile->Name = Serialization::DeserializeStdString(iStream);
		out_tile->DisplayTexture = Serialization::DeserializeStdString(iStream);
		out_tile->pattern = Serialization::DeserializeTilePattern(iStream);

		const std::string fileEndingCheck = Serialization::DeserializeStdString(iStream);
		bool valid = fileEndingCheck == FileEnding;
		if (!valid) {
			delete out_tile;
			std::cout << "File ending check failed." << std::endl;
		}

		return valid;
	}

	void Tile::Serialize(std::ostream& oStream) const {
		Serialization::Serialize(oStream, Name);
		Serialization::Serialize(oStream, DisplayTexture);
		Serialization::Serialize(oStream, pattern);
		Serialization::Serialize(oStream, FileEnding);
	}

	// Menu for Tile Creation; Returns True on success
	bool Tile::ImGuiCreateTile(bool& displayWindow, Tile*& out_tile) {
		using namespace ImGui;

		bool created = false;
		if (Begin("New Tile", &displayWindow)) {
			static char nameBuff[256];
			static char texPathBuff[1024];
			static unsigned int texID = 0;

			ImGui::InputTextWithHint("Name", "<enter tile name>", nameBuff, IM_ARRAYSIZE(nameBuff));
			ImGuiHelper::Image(texID, ImVec2(32, 32));
			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Texture")) {
					const Rendering::Texture* t = *static_cast<const Rendering::Texture**>(payload->Data);
					texID = t->GetTextureID();
					strcpy_s(texPathBuff, t->GetRelativeFilePath().c_str());
				}
				ImGui::EndDragDropTarget();
			}

			SameLine();
			constexpr char popupWindow[] = "Select Sprite";
			if (ImGui::Button("Select...")) {
				OpenPopup(popupWindow);
			}

			if (BeginPopup(popupWindow)) {
				const auto t = Resources::GetTextures();
				for (auto it = Resources::GetTextures().begin(); it != Resources::GetTextures().end(); ++it) {
					const auto& tex = it->second;
					const auto textureID = tex->GetTextureID();
					if (ImageButton((void*)textureID, ImVec2(32, 32), ImVec2(0, 1), ImVec2(1, 0))) {
						texID = textureID;
						strcpy_s(texPathBuff, it->first.c_str());
						CloseCurrentPopup();
					}
					Text(it->second->GetFileName().c_str());
				}
				EndPopup();
			}

			bool canCreate = true;
			if (nameBuff[0] == '\0') canCreate = false;
			if (texPathBuff[0] == '\0') canCreate = false;

			if (!canCreate) BeginDisabled();
			if (ImGui::Button("Create Tile")) {
				out_tile = new Tile();
				out_tile->Name = nameBuff;
				out_tile->DisplayTexture = texPathBuff;

				std::cout << "Created Tile: " << out_tile->Name << " with Tex: " << out_tile->DisplayTexture << std::endl;

				//clear buffers for next tile
				sprintf_s(nameBuff, "");
				sprintf_s(texPathBuff, "");
				texID = 0;

				created = true;
			}
			if (!canCreate) EndDisabled();
		}
		End();

		return created;
	}

	bool Tile::ImGuiEditTile(Tile* tempFile) {
		static bool hasError = false;
		static std::string errorMessage;
		static bool fileNameAlreadyExists = false;

		if (ImGui::InputTextWithHint("Name", "<enter tile name>", &tempFile->Name)) {
			std::error_code error;
			fileNameAlreadyExists = std::filesystem::exists(Files::GetRelativePathTo(tempFile), error);
			hasError = error.value() != 0;
			errorMessage = error.message();
		}


		unsigned int texId = 0;
		if (Rendering::Texture* t; Resources::TryGetTexture(tempFile->DisplayTexture.c_str(), t)) texId = t->GetTextureID();
		ImGuiHelper::TextWithToolTip("Tile Icon", "Texture that will represent this tile in menus");
		ImGuiHelper::Image(texId, ImVec2(32, 32));
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Texture")) {
				const Rendering::Texture* t = *static_cast<const Rendering::Texture**>(payload->Data);
				tempFile->DisplayTexture = t->GetRelativeFilePath();
			}
			ImGui::EndDragDropTarget();
		}

		ImGuiHelper::TextWithToolTip("Tile Textures", "Drag and drop textures from 'Sprites' folder or any texture sheet");

		tempFile->pattern.DearImGuiEditPattern();

		if (hasError) ImGui::Text(("Error: " + errorMessage).c_str());
		if (fileNameAlreadyExists) ImGui::Text("Tile with same name already exists.");

		bool disableSave = tempFile->Name.empty() || hasError || fileNameAlreadyExists;
		if (disableSave) ImGui::BeginDisabled();

		if (ImGui::Button("Save")) {
			if (Name != tempFile->Name) {
				Files::RenameFile(this, tempFile->Name);
			}
			//move tempfile into this one
			if (this != tempFile) *this = std::move(*tempFile);
			Files::SaveToFile(this);
			fileNameAlreadyExists = false;
			hasError = false;

			return true;
		}
		if (disableSave) ImGui::EndDisabled();


		return false;
	}

}