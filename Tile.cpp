#include "Tile.h"

#include <fstream>
#include <iostream>

#include "imgui.h"
#include "ImGuiHelper.h"
#include "Resources.h"
#include "Serialization.h"
#include "Texture.h"

namespace Tiles {
	bool Tile::Deserialize(std::istream& iStream, Tile*& out_tile) {
		out_tile = new Tile();

		out_tile->Name = Serialization::Deserialize(iStream);
		out_tile->Texture = Serialization::Deserialize(iStream);
		std::string fileEndingCheck = Serialization::Deserialize(iStream);

		bool valid = fileEndingCheck == fileEnding;
		if (!valid) {
			delete out_tile;
			std::cout << "File ending check failed." << std::endl;
		}

		return valid;
	}

	std::ostream& Tile::Serialize(std::ostream& oStream) {
		Serialization::Serialize(oStream, Name);
		Serialization::Serialize(oStream, Texture);
		Serialization::Serialize(oStream, fileEnding);

		return oStream;
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
					strcpy_s(texPathBuff, t->GetFilePath().c_str());
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
				out_tile->Texture = texPathBuff;

				std::cout << "Created Tile: " << out_tile->Name << " with Tex: " << out_tile->Texture << std::endl;

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


}
