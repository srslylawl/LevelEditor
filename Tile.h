#pragma once
#include <string>
#include <glm/vec2.hpp>
#include "Strings.h"
#include "TilePatterns.h"

namespace  Tiles {
	class TileMap;

	enum class TileType {
		Simple,
		AutoTile,
		AutoWall
	};

	class Tile {
		std::unique_ptr<ITilePattern> patternUPTR;
		void SetPatternFromType() {
			switch(Type) {
			case TileType::Simple:
				patternUPTR = std::make_unique<SimpleTilePattern>();
				break;
			case TileType::AutoTile:
				patternUPTR = std::make_unique<AutoTilePattern>();
				break;
			case TileType::AutoWall:
				break;
			}
		}
	public:
		Tile(const Tile& other) : DisplayTexture(other.DisplayTexture), Name(other.Name), Type(other.Type) {
			patternUPTR = other.patternUPTR->Clone();
		}
		Tile& operator=(const Tile& other) {
			DisplayTexture = other.DisplayTexture;
			Name = other.Name;
			Type = other.Type;
			patternUPTR = other.patternUPTR->Clone();
			return *this;
		}


		Tile(Tile&& other) = default;
		Tile& operator=(Tile&& other) = default;
		Tile() { SetPatternFromType(); }
		~Tile() = default;

		inline static const std::string FileEnding = ".tile";
		inline static const std::string ParentDirectory = Strings::Directory_Tiles;

		std::string DisplayTexture;
		std::string Name;
		TileType Type = TileType::Simple;

		const ITilePattern* GetPattern() const { return patternUPTR.get(); }

		void TileMapSet(TileMap* tileMap, glm::vec2 position) const;
		void TileMapErase(TileMap* tileMap, glm::vec2 position) const;

		static bool Deserialize(std::istream& iStream, Tile*& out_tile);
		void Serialize(std::ostream& oStream) const;

		bool ImGuiEditTile(Tile* tempFile);

	};
}

