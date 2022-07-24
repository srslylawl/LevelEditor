#pragma once
#include <glm/vec2.hpp>

#include "Texture.h"

namespace Rendering {
	class Texture;
}

namespace Tiles {
	class TileMap;
	class Tile;

	// Is stored in a tileMap, has reference to its parent tile.
	class TileInstance {
		const Tile* parent;
		Rendering::Texture* texture;

		bool HasSameTileAtPos(glm::ivec2 position, const TileMap* tileMap) const;
		void Refresh(glm::ivec2 position, TileMap*& tileMap);
		Rendering::Texture* GetTextureFromSurroundingTiles(glm::ivec2 position, const TileMap* tileMap) const;

	public:
		TileInstance(const Tile* parent, const glm::vec2 position, const TileMap* tileMap) : parent(parent), texture(GetTextureFromSurroundingTiles(position, tileMap)) {}

		static void RefreshSurroundingTileInstances(glm::ivec2 position, TileMap* tileMap);
		unsigned int GetActiveTextureId() const { return texture->GetTextureID(); }

	};
}

