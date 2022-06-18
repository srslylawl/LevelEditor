#include "Tile.h"

#include <fstream>
#include <iostream>

#include "imgui.h"
#include "Resources.h"
#include "Serialization.h"

namespace Tiles {
	Tile* Tile::Deserialize(std::istream& iStream) {
		auto t = new Tile();
		t->Name = Serialization::Deserialize(iStream);
		t->Texture = Serialization::Deserialize(iStream);

		return t;
	}

	std::ostream& Tile::Serialize(std::ostream& oStream) {
		Serialization::Serialize(oStream, Name);
		Serialization::Serialize(oStream, Texture);

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
			ImGui::Image((void*)texID, ImVec2(32, 32), ImVec2(0, 1), ImVec2(1, 0));
			SameLine();
			constexpr char popupWindow[] = "Select Sprite";
			if (ImGui::Button("Select...")) {
				OpenPopup(popupWindow);
			}

			if (BeginPopup(popupWindow)) {
				for (auto it = Resources::Textures.begin(); it != Resources::Textures.end(); ++it) {
					const auto& tex = it->second;
					if(ImageButton((void*)tex.ID, ImVec2(32, 32), ImVec2(0, 1), ImVec2(1, 0))) {
						
						texID = tex.ID;
						strcpy_s(texPathBuff, it->first.c_str());
						CloseCurrentPopup();
					}
					Text(it->second.name.c_str());
				}
				EndPopup();
			}


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
		}
		End();

		return created;
	}


}
