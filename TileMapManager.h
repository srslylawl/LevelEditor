#pragma once
#include <vector>

#include "TileMap.h"

namespace GridTools {
	class GridToolBar;
}

namespace Tiles {

	// Holds tileMaps and renders them in user defined order.
	class TileMapManager : public Rendering::Renderable {
	public:
		explicit TileMapManager(GridTools::GridToolBar* grid_tool_bar);

		std::vector<TileMap*> tileMaps;
		GridTools::GridToolBar* gridToolBar;
		TileMap* activeTileMap = nullptr;

		bool autoSelectTileMapOnTileSelect = false;

		void RenderImGuiWindow();

		void Render() const override;

		~TileMapManager() override {
			for (const auto& tm : tileMaps) {
				delete tm;
			}
		}
	};

}
