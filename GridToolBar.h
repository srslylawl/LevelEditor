#pragma once
#include <memory>
#include <unordered_map>
#include <glm/vec2.hpp>

#include "GridToolType.h"

namespace Tiles {
	class Tile;
	class TileMap;
}

class InputMouseEvent;

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
		GridToolBar(GridToolBar&& other) = delete;
		GridToolBar(GridToolBar& other) = delete;
		GridToolBar operator=(GridToolBar& other) = delete;
		GridToolBar operator=(GridToolBar&& other) = delete;
		GridToolBar();
		~GridToolBar();

		TileMap* activeTileMap = nullptr;


		void SelectTool(GridToolType type);
		bool OnMouseEvent(const InputMouseEvent* event);

		GridToolType GetActiveTool() const;

		glm::ivec2 GetMouseGridPos() const;

		const Tile* GetSelectedTile() const;

		void SetActiveTileMap(TileMap* tileMap) { activeTileMap = tileMap; }

		void SetSelectedTile(Tile* tile);
	};


}
