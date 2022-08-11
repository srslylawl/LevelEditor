#include "Tile.h"

#include <fstream>
#include <iostream>

#include "imgui.h"
#include "ImGuiHelper.h"
#include "Resources.h"
#include "Serialization.h"
#include "Texture.h"
#include "TileMap.h"

namespace Tiles {
	void Tile::TileMapSet(TileMap* tileMap, glm::vec2 position) const {
		tileMap->SetTile(this, position);
	}

	void Tile::TileMapErase(TileMap* tileMap, glm::vec2 position) const {
		//*this can be null here
		tileMap->RemoveTile(position);
	}

	bool Tile::Deserialize(std::istream& iStream, Tile*& out_tile) {
		AssetHeader header;
		if(!AssetHeader::Deserialize(iStream, &header)) {
			std::cout << "Unable to read header of tile" << std::endl;
			return false;
		}
		out_tile = new Tile();
		out_tile->AssetId = header.aId;
		out_tile->Name = Serialization::DeserializeStdString(iStream);
		if(!Serialization::TryDeserializeAssetId(iStream, out_tile->DisplayTexture)) {
			delete out_tile;
			return false;
		}
		int tileType = 0; Serialization::readFromStream(iStream, tileType);
		out_tile->Type = static_cast<TileType>(tileType);
		out_tile->patternUPtr = Serialization::DeserializeITilePattern(iStream);

		const std::string fileEndingCheck = Serialization::DeserializeStdString(iStream);
		bool valid = fileEndingCheck == GetFileEnding();
		if (!valid) {
			delete out_tile;
			std::cout << "File ending check failed." << std::endl;
		}

		return valid;
	}

	void Tile::Serialize(std::ostream& oStream) const {
		AssetHeader::Write(oStream, AssetType::Tile, AssetId);
		Serialization::Serialize(oStream, Name);
		Serialization::Serialize(oStream, DisplayTexture);
		Serialization::writeToStream(oStream, static_cast<int>(Type));
		Serialization::Serialize(oStream, patternUPtr.get());
		Serialization::Serialize(oStream, GetFileEnding());
	}

	bool Tile::ImGuiEditTile(Tile* tempFile) {
		static bool hasError = false;
		static std::string errorMessage;
		static bool fileNameAlreadyExists = false;

		if (ImGui::InputTextWithHint("Name", "<enter tile name>", &tempFile->Name)) {
			std::error_code error;
			fileNameAlreadyExists = std::filesystem::exists(tempFile->GetRelativePath(), error);
			hasError = error.value() != 0;
			errorMessage = error.message();
		}

		const char* const items[] = { "Simple", "AutoTile", "AutoWall" };
		if (ImGui::Combo("Type", (int*)&tempFile->Type, items, 3)) {
			tempFile->SetPatternFromType();
		}
		ImGui::SameLine(); ImGuiHelper::TextWithToolTip("", "AutoTiles and walls will automatically change their displayed texture based on the tiles around them.");

		unsigned int texId = 0;
		if (Rendering::Texture* t; Resources::TryGetTexture(DisplayTexture, t)) texId = t->GetTextureID();
		ImGuiHelper::TextWithToolTip("Tile Icon", "Texture that will represent this tile in menus");
		ImGuiHelper::Image(texId, ImVec2(32, 32));
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Texture")) {
				const Rendering::Texture* t = *static_cast<const Rendering::Texture**>(payload->Data);
				tempFile->DisplayTexture = t->AssetId;
			}
			ImGui::EndDragDropTarget();
		}

		
		ImGuiHelper::TextWithToolTip("Tile Textures", "Drag and drop textures from 'Sprites' folder or any texture sheet");

		tempFile->patternUPtr->RenderDearImGui();

		ImGui::Separator();
		if (hasError) ImGui::TextUnformatted(("Error: " + errorMessage).c_str());
		if (fileNameAlreadyExists) ImGui::TextUnformatted("Tile with same name already exists.");

		bool disableSave = tempFile->Name.empty() || hasError || fileNameAlreadyExists;
		if (disableSave) ImGui::BeginDisabled();

		if (ImGui::Button("Save")) {
			if (Name != tempFile->Name) {
				this->Rename(tempFile->Name);
			}
			//move tempfile into this one
			if (this != tempFile) *this = std::move(*tempFile);
			SaveToFile();
			fileNameAlreadyExists = false;
			hasError = false;

			return true;
		}
		if (disableSave) ImGui::EndDisabled();
		ImGui::SameLine();
		if (ImGui::Button("Cancel")) return true;


		return false;
	}

}