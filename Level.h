#pragma once
#include "Assets.h"

namespace Tiles {
	class TileMapManager;
}
class Level : public PersistentAsset<Level> {

public:
	explicit Level(std::string name);
	std::unique_ptr<Tiles::TileMapManager> TileMapManagerUPtr;

	bool isDirty = false;

	static Level* CreateDefaultLevel();

	static bool Deserialize(std::istream& iStream, const AssetHeader& header, Level*& out_Level);
	void Serialize(std::ostream& oStream) const override;
};
