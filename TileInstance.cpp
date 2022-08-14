#include "TileInstance.h"

#include "Resources.h"
#include "Tile.h"
#include "TileMap.h"

namespace Tiles {

	bool TileInstance::HasSameTileAtPos(const glm::ivec2 position, const TileMap* tileMap) const {
		TileInstance* tileInstance = nullptr;
		return tileMap->TryGetTile(position, tileInstance) && tileInstance->parent == this->parent;
	}

	SurroundingTileFlags TileInstance::GetMaskFromSurroundingTiles(const glm::ivec2 position, const TileMap* tileMap) const {
		SurroundingTileFlags surroundingMask = SurroundingTileFlags::NONE;

		if (HasSameTileAtPos(position + glm::ivec2(-1, 1), tileMap)) surroundingMask = surroundingMask | SurroundingTileFlags::UP_LEFT;
		if (HasSameTileAtPos(position + glm::ivec2(0, 1), tileMap)) surroundingMask = surroundingMask | SurroundingTileFlags::UP;
		if (HasSameTileAtPos(position + glm::ivec2(1, 1), tileMap)) surroundingMask = surroundingMask | SurroundingTileFlags::UP_RIGHT;
		if (HasSameTileAtPos(position + glm::ivec2(-1, 0), tileMap)) surroundingMask = surroundingMask | SurroundingTileFlags::LEFT;
		if (HasSameTileAtPos(position + glm::ivec2(1, 0), tileMap)) surroundingMask = surroundingMask | SurroundingTileFlags::RIGHT;
		if (HasSameTileAtPos(position + glm::ivec2(-1, -1), tileMap)) surroundingMask = surroundingMask | SurroundingTileFlags::DOWN_LEFT;
		if (HasSameTileAtPos(position + glm::ivec2(0, -1), tileMap)) surroundingMask = surroundingMask | SurroundingTileFlags::DOWN;
		if (HasSameTileAtPos(position + glm::ivec2(1, -1), tileMap)) surroundingMask = surroundingMask | SurroundingTileFlags::DOWN_RIGHT;

		return surroundingMask;
	}

	Rendering::Texture* TileInstance::GetTextureFromMask(const glm::ivec2 position) const {
		Rendering::Texture* t = Rendering::Texture::Empty();
		auto slot = parent->GetPattern()->GetTileSlot(surroundingTileMask);
		if (slot) {
			const TextureVariant& variant = slot->TileSprites.back(); //TODO: get by probability and position instead
			Resources::TryGetTexture(variant.TextureId, t);
		}

		return t;
	}

	void TileInstance::Refresh(glm::ivec2 position, const TileMap* tileMap) {
		auto mask = parent->TileType == TileType::Simple ? SurroundingTileFlags::NONE : GetMaskFromSurroundingTiles(position, tileMap);
		surroundingTileMask = mask;
		texture = GetTextureFromMask(position);
	}


}
