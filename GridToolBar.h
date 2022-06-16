#pragma once
#include <unordered_map>
#include "Camera.h"
#include "GridToolType.h"
#include "TileMap.h"

namespace GridTools {
	class GridTool;
	using namespace Tiles;

	class GridToolBar {
		std::unordered_map<GridToolType, GridTool*> tools;

		GridToolType activeTool = GridToolType::Place;
		bool isStale = false; //did either the tile, tool or grid position change since last placement?
		Tile* selectedTile = nullptr;
		glm::ivec2 lastMouseGridPos = { INT_MAX, INT_MAX };

	public:
		TileMap* tileMap;

		explicit GridToolBar(TileMap* tile_map);
		~GridToolBar();

		void SelectTool(GridToolType type);
		void OnMouseEvent(const InputMouseEvent* event);
		glm::ivec2 GetMouseGridPos() const;
		const Tile* GetSelectedTile() const;

		void SetSelectedTile(Tile* tile);
	};

	
}
