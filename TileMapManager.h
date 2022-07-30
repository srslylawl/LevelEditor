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
		TileMapManager() {}

		std::vector<TileMap*> tileMaps;
		GridTools::GridToolBar* gridToolBar = nullptr;
		TileMap* activeTileMap = nullptr;

		bool autoSelectTileMapOnTileSelect = false;

		void RenderImGuiWindow();

		void Render() const override;

		void SetActiveTileMap(TileMap* tileMap);

		~TileMapManager() override {
			for (const auto& tm : tileMaps) {
				delete tm;
			}
		}
	};

}
