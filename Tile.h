#pragma once
#include <string>
#include <vector>

#include "Strings.h"

namespace  Tiles {

	struct TileSprite {
		std::string Texture;
		float ProbabilityModifier;
	};

	struct TileSlot {
		std::vector<TileSprite> TileSprites;
	};
	class Tile {
	public:
		std::string Texture; //TODO: change this from string to something else
		std::string Name;
		inline static const std::string FileEnding = ".tile";
		inline static const std::string ParentDirectory = Strings::Directory_Tiles;

		static bool Deserialize(std::istream& iStream, Tile*& out_tile);
		void Serialize(std::ostream& oStream) const;

		static bool ImGuiCreateTile(bool& displayWindow, Tile*& out_tile);
	};
}

