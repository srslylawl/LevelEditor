#include "GridToolBar.h"
#include "GridTool.h"


namespace GridTools {
	void PlacerTool::OnInteract(const InputMouseEvent* event, const glm::ivec2& position, bool& isStale) {
		if (event->GetMouseKeyHold(MouseButton::Left) && !isStale) {
			toolBar->tileMap->SetTile(toolBar->GetSelectedTile(), position);
			isStale = true;
		}
	}

	void EraserTool::OnInteract(const InputMouseEvent* event, const glm::ivec2& position, bool& isStale) {
		if(event->GetMouseKeyHold(MouseButton::Left) && !isStale) {
			toolBar->tileMap->RemoveTile(position);
			isStale = true;
		}
	}
}
