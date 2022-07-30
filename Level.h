#pragma once
#include <istream>
#include "Strings.h"

namespace Tiles {
	class TileMapManager;
}
class Level {

public:
	explicit Level(std::string name);
	std::unique_ptr<Tiles::TileMapManager> TileMapManagerUPTR;

	inline static const std::string FileEnding = ".level";
	inline static const std::string ParentDirectory = Strings::Directory_Levels;
	std::string Name = "untitled";

	bool isDirty = false;

	static Level* CreateDefaultLevel();

	static bool Deserialize(std::istream& iStream, Level*& out_Level);
	void Serialize(std::ostream& oStream) const;

};

