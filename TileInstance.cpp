#include "TileInstance.h"

#include "Resources.h"
#include "Tile.h"
#include "TileMap.h"

namespace Tiles {

	bool TileInstance::HasSameTileAtPos(const glm::ivec2 position, const TileMap* tileMap) const {
		const TileInstance* tileInstance = nullptr;
		return tileMap->TryGetTile(position, tileInstance) && tileInstance->parent == this->parent;
	}

	void TileInstance::RefreshSurroundingTileInstances(const glm::ivec2 position, TileMap* tileMap) {
		TileInstance* t = nullptr;
		for(int x = -1; x <= 1; ++x) {
			for(int y = -1; y <=1; ++y) {
				if(x == 0 && y == 0) continue; //skip self
				auto tilePos = glm::ivec2(position.x + x, position.y + y);
				if(tileMap->TryGetTile(tilePos, t)) t->Refresh(tilePos, tileMap);
			}
		}
	}

	Rendering::Texture* TileInstance::GetTextureFromSurroundingTiles(const glm::ivec2 position, const TileMap* tileMap) const {
		Rendering::Texture* t = Rendering::Texture::Empty();

		//Check all surrounding tiles and build mask
		SurroundingTileFlags surroundingMask = SurroundingTileFlags::NONE;
		if (HasSameTileAtPos(position + glm::ivec2(-1, 1), tileMap)) surroundingMask = surroundingMask | SurroundingTileFlags::UP_LEFT;
		if (HasSameTileAtPos(position + glm::ivec2(0, 1), tileMap)) surroundingMask = surroundingMask | SurroundingTileFlags::UP;
		if (HasSameTileAtPos(position + glm::ivec2(1, 1), tileMap)) surroundingMask = surroundingMask | SurroundingTileFlags::UP_RIGHT;
		if (HasSameTileAtPos(position + glm::ivec2(-1, 0), tileMap)) surroundingMask = surroundingMask | SurroundingTileFlags::LEFT;
		if (HasSameTileAtPos(position + glm::ivec2(1, 0), tileMap)) surroundingMask = surroundingMask | SurroundingTileFlags::RIGHT;
		if (HasSameTileAtPos(position + glm::ivec2(-1, -1), tileMap)) surroundingMask = surroundingMask | SurroundingTileFlags::DOWN_LEFT;
		if (HasSameTileAtPos(position + glm::ivec2(0, -1), tileMap)) surroundingMask = surroundingMask | SurroundingTileFlags::DOWN;
		if (HasSameTileAtPos(position + glm::ivec2(1, -1), tileMap)) surroundingMask = surroundingMask | SurroundingTileFlags::DOWN_RIGHT;
		auto slot = parent->GetPattern()->GetTileSlot(surroundingMask);
		if(slot) {
			auto variant = slot->TileSprites.back(); //TODO: get by probability and position instead
			Resources::TryGetTexture(variant.Texture.c_str(), t);
		}

		return t;
	}

	void TileInstance::Refresh(glm::ivec2 position, TileMap*& tileMap) {
		texture = GetTextureFromSurroundingTiles(position, tileMap);
	}


}
