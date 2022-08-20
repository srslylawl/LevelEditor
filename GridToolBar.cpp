#include "GridToolBar.h"

#include <iostream>

#include "Camera.h"
#include "GridTool.h"
#include "Input.h"

namespace GridTools {

	GridToolBar::GridToolBar() {
		tools = {
			{GridToolType::Place, new PlacerTool(this)},
			{GridToolType::Erase, new EraserTool(this)},
			{GridToolType::Select, new class SelectTool(this)}
		};
	}

	GridToolBar::~GridToolBar() {
		for (auto& kvp : tools)
			delete kvp.second;
	}


	void GridToolBar::SelectTool(GridToolType type) {
		if (activeTool == type) return;

		if (tools.find(type) == tools.end()) {
			std::cout << "Type: " << (int)type << " not found in tools dict!" << std::endl;
			return;
		}

		tools[activeTool]->OnDeselect();
		tools[type]->OnSelect();

		activeTool = type;
		isStale = false;
	}

	bool GridToolBar::OnMouseEvent(const InputMouseEvent* event) {
		if (activeTileMap == nullptr) {
			std::cout << "No tileMap selected!" << std::endl;
			return false;
		}
		const auto gridPos = GetMouseGridPos();
		if (gridPos != lastMouseGridPos) isStale = false;

		bool success = tools[activeTool]->OnInteract(event, gridPos, isStale);

		lastMouseGridPos = gridPos;

		return success;
	}
	glm::ivec2 GridToolBar::GetMouseGridPos() const {
		const auto mousePos = Input::GetMousePosition();
		const auto mouseCoords = Rendering::Camera::Main->ScreenToGridPosition(mousePos.x, mousePos.y);
		return { floor(mouseCoords.x), floor(mouseCoords.y) };
	}

	GridToolType GridToolBar::GetActiveTool() const {
		return activeTool;
	}

	const Tile* GridToolBar::GetSelectedTile() const {
		return selectedTile;
	}
	void GridToolBar::SetSelectedTile(Tile* tile) {
		selectedTile = tile;
		isStale = false;
	}

}
