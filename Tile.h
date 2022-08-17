#pragma once
#include <glm/vec2.hpp>

#include "Assets.h"
#include "FileEditWindow.h"
#include "TilePatterns.h"

namespace  Tiles {
	class TileMap;

	enum class TileType {
		Simple,
		AutoTile,
		AutoWall
	};

	class Tile : public PersistentAsset<Tile>, public IEditable {
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
		Tile(const Tile& other) : PersistentAsset(other.AssetId, AssetType::Tile, other.ParentPath, other.Name),
		                          patternUPtr(other.patternUPtr->Clone()), DisplayTexture(other.DisplayTexture), TileType(other.TileType) {
		}
		Tile& operator=(const Tile& other) {
			PersistentAsset::operator=(other);
			DisplayTexture = other.DisplayTexture;
			TileType = other.TileType;
			patternUPtr = other.patternUPtr->Clone();
			return *this;
		}

		Tile(Tile&& other) = default;
		Tile& operator=(Tile&& other) = default;
		Tile(::AssetId assetId, const std::filesystem::path& relativeFilePath) : PersistentAsset(assetId, AssetType::Tile, relativeFilePath.parent_path(), relativeFilePath.filename().replace_extension().string()) {
			SetPatternFromType();
		}
		Tile() : Tile(AssetId::CreateNewAssetId(), "") {
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

		bool RenderEditWindow(FileEditWindow* editWindow, bool isNewFile) override;
		static std::unique_ptr<IEditable> CreateNew(std::filesystem::path directory){
			auto ptr = std::make_unique<Tile>();
			ptr->ParentPath = std::move(directory);
			return ptr;
		}

		std::filesystem::path IEditableGetAssetPath() override {
			return GetRelativeAssetPath();
		}

	};

}
