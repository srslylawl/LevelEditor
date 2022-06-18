#pragma once
#include <ostream>
#include <string>

namespace  Tiles {
	class Tile {
	public:
		std::string Texture; //TODO: change this from string to something else
		std::string Name;

		static Tile* Deserialize(std::istream& iStream);
		std::ostream& Serialize(std::ostream& oStream);

		static bool ImGuiCreateTile(bool& displayWindow, Tile*& out_tile);
	};
}

