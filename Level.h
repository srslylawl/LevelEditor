#pragma once
#include "Asset.h"
#include "Strings.h"

namespace Tiles {
	class TileMapManager;
}
class Level : public StandaloneAsset<Level> {

public:
	explicit Level(std::string name);
	std::unique_ptr<Tiles::TileMapManager> TileMapManagerUPTR;

	bool isDirty = false;

	static Level* CreateDefaultLevel();

	static bool Deserialize(std::istream& iStream, Level*& out_Level);
	void Serialize(std::ostream& oStream) const override;
};

template <>
inline const std::string StandaloneAsset<Level>::GetFileEnding() {
	return ".level";
}

template <>
inline const std::string StandaloneAsset<Level>::GetParentDirectory() {
	return Strings::Directory_Levels;
}

