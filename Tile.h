#pragma once
#include <string>
#include <glm/vec2.hpp>

#include "Assets.h"
#include "Strings.h"
#include "TilePatterns.h"

namespace  Tiles {
	class TileMap;

	enum class TileType {
		Simple,
		AutoTile,
		AutoWall
	};

	class Tile : public PersistentAsset<Tile> {
		std::unique_ptr<ITilePattern> patternUPtr;
		void SetPatternFromType() {
			switch (TileType) {
				case TileType::Simple:
					patternUPtr = std::make_unique<SimpleTilePattern>();
					break;
				case TileType::AutoTile:
					patternUPtr = std::make_unique<AutoTilePattern>();
					break;
				case TileType::AutoWall:
					break;
			}
		}
	public:
		Tile(const Tile& other) : PersistentAsset(other.AssetId, AssetType::Tile, other.Name), DisplayTexture(other.DisplayTexture), TileType(other.TileType){
			Name = other.Name;
			patternUPtr = other.patternUPtr->Clone();
		}
		Tile& operator=(const Tile& other) {
			DisplayTexture = other.DisplayTexture;
			Name = other.Name;
			TileType = other.TileType;
			patternUPtr = other.patternUPtr->Clone();
			return *this;
		}


		Tile(Tile&& other) = default;
		Tile& operator=(Tile&& other) = default;
		Tile(::AssetId assetId, const std::string& name) : PersistentAsset(assetId, AssetType::Tile, name) {
			SetPatternFromType();
		}
		Tile() : Tile(::AssetId(), "") {
			SetPatternFromType();
		}
		~Tile() override = default;

		::AssetId DisplayTexture;
		TileType TileType = TileType::Simple;

		const ITilePattern* GetPattern() const {
			return patternUPtr.get();
		}

		void TileMapSet(TileMap* tileMap, glm::vec2 position) const;
		void TileMapErase(TileMap* tileMap, glm::vec2 position) const;

		void Serialize(std::ostream& oStream) const override;
		static bool Deserialize(std::istream& iStream, const AssetHeader& header, Tile*& out_tile);

		bool ImGuiEditTile(Tile* tempFile);
	};

}
template<> inline const std::string PersistentAsset<Tiles::Tile>::GetFileEnding() {
	return ".tile";
}
template<> inline const std::string PersistentAsset<Tiles::Tile>::GetParentDirectory() {
	return Strings::Directory_Tiles;
}

