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

	bool Tile::Deserialize(std::istream& iStream, const AssetHeader& header, Tile*& out_tile) {
		out_tile = new Tile(header.aId, header.relativeAssetPath);
		if (!Serialization::TryDeserializeAssetId(iStream, out_tile->DisplayTexture)) {
			delete out_tile;
			return false;
		}
		int tileType = 0; Serialization::readFromStream(iStream, tileType);
		out_tile->TileType = static_cast<Tiles::TileType>(tileType);
		out_tile->patternUPtr = Serialization::DeserializeITilePattern(iStream);
		return true;
	}

	void Tile::Serialize(std::ostream& oStream) const {
		Serialization::Serialize(oStream, DisplayTexture);
		Serialization::writeToStream(oStream, static_cast<int>(TileType));
		Serialization::Serialize(oStream, patternUPtr.get());
	}

	bool Tile::RenderEditWindow(FileEditWindow* editWindow, bool isNewFile) {
		ImGui::InputTextWithHint("Name", "<enter tile name>", &Name);
		const char* const items[] = { "Simple", "AutoTile", "AutoWall" };
		if (ImGui::Combo("Type", (int*)&TileType, items, 3)) {
			SetPatternFromType();
		}
		ImGui::SameLine(); ImGuiHelper::TextWithToolTip("", "AutoTiles and walls will automatically change their displayed texture based on the tiles around them.");

		unsigned int texId = 0;
		if (Rendering::Texture* t; Resources::TryGetTexture(DisplayTexture, t)) texId = t->GetTextureID();
		ImGuiHelper::TextWithToolTip("Tile Icon", "Texture that will represent this tile in menus");
		ImGuiHelper::Image(texId, ImVec2(32, 32));
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Texture")) {
				const Rendering::Texture* t = *static_cast<const Rendering::Texture**>(payload->Data);
				DisplayTexture = t->AssetId;
			}
			ImGui::EndDragDropTarget();
		}
		ImGuiHelper::TextWithToolTip("Tile Textures", "Drag and drop textures from 'Sprites' folder or any texture sheet");

		patternUPtr->RenderDearImGui();

		ImGui::Separator();
		std::string errorMessage;
		const bool disableSave = !CanSave(errorMessage, true);
		if (disableSave) {
			ImGui::TextUnformatted(errorMessage.c_str());
			ImGui::BeginDisabled();
		}

		if (ImGui::Button("Save")) {
			if (!isNewFile && GetRelativeAssetPath() != editWindow->oldPath) {
				this->Rename(editWindow->oldPath, GetRelativeAssetPath());
			}
			SaveToFile();
			return true;
		}
		if (disableSave) ImGui::EndDisabled();
		ImGui::SameLine();
		if (ImGui::Button("Cancel")) {
			//reload from disk if its not a new file
			if (isNewFile) return true;

			Tile* old;
			if (!LoadFromFile(editWindow->oldPath.string().c_str(), old)) {
				std::cerr << "Unable to load old tile when discarding changes: " << editWindow->oldPath.string() << std::endl;
				return true;
			}
			*this = std::move(*old);

			return true;
		}


		return false;
	}

}
