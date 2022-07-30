#pragma once
#include <glm/vec2.hpp>

#include "Texture.h"
#include "TilePatterns.h"

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

		SurroundingTileFlags surroundingTileMask = SurroundingTileFlags::NONE;

		bool HasSameTileAtPos(glm::ivec2 position, const TileMap* tileMap) const;
		SurroundingTileFlags GetMaskFromSurroundingTiles(const glm::ivec2 position, const TileMap* tileMap) const;

		Rendering::Texture* GetTextureFromMask(glm::ivec2 position) const;

	public:
		TileInstance(const Tile* parent, const glm::vec2 position, const TileMap* tileMap) : parent(parent) {
			Refresh(position, tileMap);
		}
		TileInstance(const Tile* parent, const SurroundingTileFlags tileFlags, const glm::ivec2 position) : parent(parent), surroundingTileMask(tileFlags) { texture = GetTextureFromMask(position); }

		const Tile* GetParent() const { return parent; }
		SurroundingTileFlags GetMask() const {return surroundingTileMask; }

		void Refresh(glm::ivec2 position, const TileMap* tileMap);
		unsigned int GetActiveTextureId() const { return texture->GetTextureID(); }

	};
}

