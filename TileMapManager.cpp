#include "TileMapManager.h"
#include "ImGuiHelper.h"
#include "GridToolBar.h"

void Tiles::TileMapManager::RenderImGuiWindow() {
	using namespace ImGui;
	static bool tileMapWindowOpen = true;
	const auto main_viewport = GetMainViewport();

	SetNextWindowSize(ImVec2(300, 200), ImGuiCond_Once);
	PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(200, 200));
	if (Begin("TileMaps", &tileMapWindowOpen, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove)) {
		ImGuiHelper::TextCentered("Active TileMap");

		int tileMapCount = activeTileMaps.size();
		static int selectedIndex = 0;
		bool reorder = false;
		int reorderFirst = 0;
		int reorderSecond = 0;
		if (BeginTable("###TileMapListBox", 5, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingFixedFit| ImGuiTableFlags_ScrollY, ImVec2(GetWindowSize().x-30, 150))) {
			TableSetupColumn("id", ImGuiTableColumnFlags_WidthFixed, 10);
			TableSetupColumn("name", ImGuiTableColumnFlags_WidthStretch, 50);
			TableSetupColumn("-", ImGuiTableColumnFlags_WidthFixed, 10);
			TableSetupColumn("+", ImGuiTableColumnFlags_WidthFixed, 10);
			TableSetupColumn("visibility", ImGuiTableColumnFlags_WidthFixed, 30);
			for (int i = tileMapCount - 1; i >= 0; --i) {
				bool isSelected = i == selectedIndex;
				TableNextRow();
				TableNextColumn();
				Tiles::TileMap* currentTileMap = activeTileMaps[i];
				TextUnformatted(std::to_string(i + 1).c_str());
				TableNextColumn();
				if (Selectable(currentTileMap->Name.c_str(), isSelected)) {
					selectedIndex = i;
					gridToolBar->SetActiveTileMap(currentTileMap);
				}
				TableNextColumn();
				bool allowMinus = i > 0;
				if (!allowMinus) BeginDisabled();
				std::string buttonId = "-" + std::to_string(i);
				PushID(buttonId.c_str());
				if (Button("-")) {
					reorder = true;
					reorderFirst = i;
					reorderSecond = i - 1;
					if (isSelected) selectedIndex = i - 1;
				}
				PopID();
				if (!allowMinus) EndDisabled();
				TableNextColumn();
				bool allowPlus = i < tileMapCount - 1;
				if (!allowPlus) BeginDisabled();
				buttonId = "+" + std::to_string(i);
				PushID(buttonId.c_str());
				if (Button("+")) {
					reorder = true;
					reorderFirst = i;
					reorderSecond = i + 1;
					if (isSelected) selectedIndex = i + 1;
				}
				PopID();
				if (!allowPlus) EndDisabled();
				TableNextColumn();
				PushID(i);
				Checkbox("Visible", &currentTileMap->renderingEnabled);
				if(IsItemHovered()) {
					BeginTooltip();
					TextUnformatted("Show/Hide TileMap");
					EndTooltip();
				}
				PopID();
			}
			EndTable();
		}

		if (TreeNode("More...")) {
			Checkbox("Auto-select on tile select <NYI>", &autoSelectTileMapOnTileSelect); SameLine();
			ImGuiHelper::TextWithToolTip("", "When selecting a different tile, will attempt to automatically select the matching type of tileMap");
			TreePop();
		}

		if (reorder) {
			auto temp = activeTileMaps[reorderFirst];
			activeTileMaps[reorderFirst] = activeTileMaps[reorderSecond];
			activeTileMaps[reorderSecond] = temp;
		}
		float xPos = main_viewport->Size.x - ImGui::GetWindowSize().x;
		float yPos = main_viewport->Size.y - main_viewport->WorkSize.y;
		ImGui::SetWindowPos(ImVec2(xPos, yPos));
	}
	ImGui::End();
	ImGui::PopStyleVar();
}
