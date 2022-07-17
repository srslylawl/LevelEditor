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

	void Tile::Update(glm::ivec2 position, TileMap tileMap) {

	}

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

	bool Tile::ImGuiEditTile(bool creatingNew) {
		if (NewTileName.empty()) NewTileName = Name;

		static bool nameChanged = false;
		static bool fileNameAlreadyExists = false;
		static bool hasError = false;
		static std::string errorMessage;
		if (ImGui::InputTextWithHint("Name", "<enter tile name>", &NewTileName)) {
			std::error_code error;
			fileNameAlreadyExists = std::filesystem::exists(Files::GetRelativePathTo(this, NewTileName), error);
			hasError = error.value() != 0;
			errorMessage = error.message();
			nameChanged = true;
		}


		unsigned int texId = 0;
		if (Rendering::Texture* t; Resources::TryGetTexture(DisplayTexture.c_str(), t)) texId = t->GetTextureID();
		ImGui::Text("Tile Icon");
		ImGuiHelper::Image(texId, ImVec2(32, 32));
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Texture")) {
				const Rendering::Texture* t = *static_cast<const Rendering::Texture**>(payload->Data);
				DisplayTexture = t->GetRelativeFilePath();
			}
			ImGui::EndDragDropTarget();
		}

		pattern.DearImGuiEditPattern();

		if(hasError) {
			ImGui::Text(("Error: " + errorMessage).c_str());
		}

		if (nameChanged && fileNameAlreadyExists) {
			ImGui::Text("Tile with same name already exists.");
			ImGui::BeginDisabled();
		}
		else if (NewTileName.empty() || hasError) ImGui::BeginDisabled();

		if (ImGui::Button("Save")) {
			if (!creatingNew && nameChanged) {
				Files::Rename(this, NewTileName);
			}
			Name = NewTileName;
			NewTileName = "";
			Files::SaveToFile(this);

			nameChanged = false;
			fileNameAlreadyExists = false;
			hasError = false;
			return true;
		}
		if (nameChanged && fileNameAlreadyExists || NewTileName.empty() || hasError) ImGui::EndDisabled();

		return false;
	}


}
