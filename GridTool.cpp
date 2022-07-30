#include "GridToolBar.h"
#include "Input.h"

#include "GridTool.h"

#include "TileMap.h"
#include <iostream>

#include "Tile.h"


namespace GridTools {
	bool PlacerTool::OnInteract(const InputMouseEvent* event, const glm::ivec2& position, bool& isStale) {
		if (event->GetMouseKeyHold(MouseButton::Left) && !isStale) {
			auto t = toolBar->GetSelectedTile();
			if(t == nullptr) {
				std::cout << "Selected Tile is NULL!" << std::endl;
				return false;
			}
			t->TileMapSet(toolBar->activeTileMap, position);
			isStale = true;

			return true;
		}

		return false;
	}

	bool EraserTool::OnInteract(const InputMouseEvent* event, const glm::ivec2& position, bool& isStale) {
		if(event->GetMouseKeyHold(MouseButton::Left) && !isStale) {
			//get selected tile can be null here, but thats ok since we check for it in tilemap erase
			toolBar->GetSelectedTile()->TileMapErase(toolBar->activeTileMap, position);
			isStale = true;
			return true;
		}

		return false;
	}
}
