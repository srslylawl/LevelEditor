#pragma once
#include <string>
#include <glm/vec2.hpp>

#include "Strings.h"
#include "TilePattern.h"

namespace  Tiles {
	class TileMap;

	class Tile {
	public:
		inline static const std::string FileEnding = ".tile";
		inline static const std::string ParentDirectory = Strings::Directory_Tiles;

		std::string DisplayTexture; //TODO: change this from string to something else
		std::string Name;

		TilePattern pattern;

		void Update(glm::ivec2 position, TileMap tileMap);

		static bool Deserialize(std::istream& iStream, Tile*& out_tile);
		void Serialize(std::ostream& oStream) const;

		static bool ImGuiCreateTile(bool& displayWindow, Tile*& out_tile);
		bool ImGuiEditTile(bool creatingNew = false);

		inline static std::string NewTileName = "";
	};
}

