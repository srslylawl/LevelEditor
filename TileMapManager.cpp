#include "TileMapManager.h"

#include "Camera.h"
#include "ImGuiHelper.h"
#include "GridToolBar.h"
#include "Input.h"
#include "Mesh.h"
#include "Renderer.h"
#include "Shader.h"

Tiles::TileMapManager::TileMapManager(GridTools::GridToolBar* grid_tool_bar) : gridToolBar(grid_tool_bar) {
}

void Tiles::TileMapManager::RenderImGuiWindow() {
	using namespace ImGui;
	static bool tileMapWindowOpen = true;
	const auto main_viewport = GetMainViewport();

	SetNextWindowSize(ImVec2(300, 200), ImGuiCond_Once);
	PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(200, 200));
	if (Begin("TileMaps", &tileMapWindowOpen, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove)) {
		ImGuiHelper::TextCentered("Active TileMap");

		int tileMapCount = tileMaps.size();
		static int selectedIndex = 0;
		bool reorder = false;
		int reorderFirst = 0;
		int reorderSecond = 0;
		if (BeginTable("###TileMapListBox", 5, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY, ImVec2(GetWindowSize().x - 30, 150))) {
			TableSetupColumn("id", ImGuiTableColumnFlags_WidthFixed, 10);
			TableSetupColumn("name", ImGuiTableColumnFlags_WidthStretch, 50);
			TableSetupColumn("-", ImGuiTableColumnFlags_WidthFixed, 10);
			TableSetupColumn("+", ImGuiTableColumnFlags_WidthFixed, 10);
			TableSetupColumn("visibility", ImGuiTableColumnFlags_WidthFixed, 30);
			for (int i = tileMapCount - 1; i >= 0; --i) {
				bool isSelected = i == selectedIndex;
				TableNextRow();
				TableNextColumn();
				Tiles::TileMap* currentTileMap = tileMaps[i];
				TextUnformatted(std::to_string(i + 1).c_str());
				TableNextColumn();
				if (Selectable(currentTileMap->Name.c_str(), isSelected)) {
					selectedIndex = i;
					SetActiveTileMap(currentTileMap);
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
				if (IsItemHovered()) {
					BeginTooltip();
					TextUnformatted("Show/Hide TileMap");
					EndTooltip();
				}
				PopID();
			}
			EndTable();
		}

		//if (TreeNode("More...")) {
		//	Checkbox("Auto-select on tile select <NYI>", &autoSelectTileMapOnTileSelect); SameLine();
		//	ImGuiHelper::TextWithToolTip("", "When selecting a different tile, will attempt to automatically select the matching type of tileMap");
		//	TreePop();
		//}

		if (reorder) {
			auto temp = tileMaps[reorderFirst];
			tileMaps[reorderFirst] = tileMaps[reorderSecond];
			tileMaps[reorderSecond] = temp;
		}
		float xPos = main_viewport->Size.x - ImGui::GetWindowSize().x;
		float yPos = main_viewport->Size.y - main_viewport->WorkSize.y;
		ImGui::SetWindowPos(ImVec2(xPos, yPos));
	}
	End();
	PopStyleVar();
}

void Tiles::TileMapManager::Render() const {
	for (const auto& tileMap : tileMaps) if (tileMap->renderingEnabled) tileMap->Render();

	// render grid
	if (!Rendering::Renderer::DrawGrid) return;

	glm::ivec2 gridDimensions(1, 1);
	if (activeTileMap != nullptr) {
		gridDimensions = activeTileMap->GridDimensions;
	}

	auto mousePos = Input::GetMousePosition();
	auto mouseGridPos = Rendering::Camera::Main->ScreenToGridPosition(mousePos.x, mousePos.y);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	const auto& gridShader = Rendering::Renderer::gridShader;
	gridShader->Use();
	gridShader->setMat4("view", *Rendering::Camera::Main->GetViewMatrix());
	gridShader->setMat4("projection", *Rendering::Camera::Main->GetProjectionMatrix());
	gridShader->setVec("mousePos", mouseGridPos);
	gridShader->setVec("gridDimensions", gridDimensions);
	gridShader->setMat4("model", glm::scale(glm::mat4(1.0f), glm::vec3(1000)));

	// clear depth buffer to always draw grid on top -- in this case no depth buffer is active
	// glClear(GL_DEPTH_BUFFER_BIT);
	Mesh::StaticMesh::GetDefaultQuad()->Draw();
}

void Tiles::TileMapManager::SetActiveTileMap(TileMap* tileMap) {
	activeTileMap = tileMap;
	gridToolBar->activeTileMap = tileMap;
}
