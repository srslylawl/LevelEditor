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

		explicit TileMapManager(GridTools::GridToolBar* grid_tool_bar)
			: gridToolBar(grid_tool_bar) {}

		std::vector<TileMap*> activeTileMaps;
		GridTools::GridToolBar* gridToolBar;

		bool autoSelectTileMapOnTileSelect = false;

		void RenderImGuiWindow();

		void Render() const override {
			for (const auto& tileMap : activeTileMaps) if (tileMap->renderingEnabled) tileMap->Render();
		}

		~TileMapManager() override {
			for (const auto& atm : activeTileMaps) {
				delete atm;
			}
		}
	};

}
