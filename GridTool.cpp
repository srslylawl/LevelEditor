#include "GridToolBar.h"
#include "Input.h"

#include "GridTool.h"

#include "TileMap.h"
#include <iostream>


namespace GridTools {
	void PlacerTool::OnInteract(const InputMouseEvent* event, const glm::ivec2& position, bool& isStale) {
		if (event->GetMouseKeyHold(MouseButton::Left) && !isStale) {
			auto t = toolBar->GetSelectedTile();
			if(t == nullptr) {
				std::cout << "Selected Tile is NULL!" << std::endl;
				return;
			}
			toolBar->activeTileMap->SetTile(t, position);
			isStale = true;
		}
	}

	void EraserTool::OnInteract(const InputMouseEvent* event, const glm::ivec2& position, bool& isStale) {
		if(event->GetMouseKeyHold(MouseButton::Left) && !isStale) {
			toolBar->activeTileMap->RemoveTile(position);
			isStale = true;
		}
	}
}
