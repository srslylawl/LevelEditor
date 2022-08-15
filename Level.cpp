#include "Level.h"
#include "Serialization.h"
#include "TileMapManager.h"

Level::Level(std::string name) : PersistentAsset(AssetId::CreateNewAssetId(), AssetType::Level, "") {
	TileMapManagerUPtr = std::make_unique<Tiles::TileMapManager>();
}

bool Level::Deserialize(std::istream& iStream, const AssetHeader& header, Level*& out_Level) {
	auto levelUPTR = std::make_unique<Level>("default");
	levelUPTR->Name = Serialization::DeserializeStdString(iStream);
	size_t tileMapCount = 0; Serialization::readFromStream(iStream, tileMapCount);
	for (int i = 0; i < tileMapCount; ++i) {
		Tiles::TileMap* tileMap = nullptr;
		if (Tiles::TileMap::Deserialize(iStream, tileMap)) {
			levelUPTR->TileMapManagerUPtr->tileMaps.push_back(tileMap);
		}
	}
	out_Level = levelUPTR.release();
	return true;
}

Level* Level::CreateDefaultLevel() {
	Level* level = new Level("untitled");
	level->TileMapManagerUPtr->tileMaps.push_back(new Tiles::TileMap("Floor", Tiles::TileMapType::Floor));
	level->TileMapManagerUPtr->tileMaps.push_back(new Tiles::TileMap("Wall", Tiles::TileMapType::Wall, glm::ivec2(1, 2)));
	level->TileMapManagerUPtr->tileMaps.push_back(new Tiles::TileMap("Ceiling", Tiles::TileMapType::Ceiling));

	return level;
}

void Level::Serialize(std::ostream& oStream) const {
	Serialization::Serialize(oStream, Name);

	size_t tileMapCount = TileMapManagerUPtr->tileMaps.size();
	Serialization::writeToStream(oStream, tileMapCount);
	for (const auto& tileMap : TileMapManagerUPtr->tileMaps) {
		tileMap->Serialize(oStream);
	}
}
