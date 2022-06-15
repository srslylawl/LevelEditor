#include "GridToolBar.h"
#include "GridTool.h"


namespace GridTools {
	GridToolBar::GridToolBar(TileMap* tile_map) : tileMap(tile_map){
		tools = {
			{GridToolType::Place, new PlacerTool()},
			{GridToolType::Erase, new EraserTool()}
		};
	}

	GridToolBar::~GridToolBar() {
		for (auto kvp : tools)
			delete kvp.second;
	}


	void GridToolBar::SelectTool(GridToolType type) {
		if (activeTool == type) return;

		if(tools.find(type) == tools.end()) {
			std::cout << "Type: " << (int)type << " not found in tools dict!" << std::endl;
			return;
		}

		tools[activeTool]->OnDeselect();
		tools[type]->OnSelect();

		activeTool = type;
		isStale = false;
	}
	void GridToolBar::OnInteract(const InputMouseEvent* event) {
		const auto gridPos = GetMouseGridPos();
		if(gridPos != lastMouseGridPos) isStale = false;

		tools[activeTool]->OnInteract(event, gridPos, isStale);

		lastMouseGridPos = gridPos;
	}
	glm::ivec2 GridToolBar::GetMouseGridPos() const {
		const auto mousePos = Input::GetMousePosition();
		const auto mouseCoords = Rendering::Camera::Main->ScreenToGridPosition(mousePos.x, mousePos.y);
		return { floor(mouseCoords.x), floor(mouseCoords.y) };
	}
	const Tile* GridToolBar::GetSelectedTile() const {
		return selectedTile;
	}
	void GridToolBar::SetSelectedTile(Tile* tile) {
		selectedTile = tile;
		isStale = false;
	}

}
