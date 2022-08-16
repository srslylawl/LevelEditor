#include "Level.h"

#include <utility>
#include "Serialization.h"
#include "TileMapManager.h"
#include "Strings.h"

Level::Level(std::string name) : PersistentAsset(AssetId::CreateNewAssetId(), AssetType::Level, Strings::Directory_Levels, std::move(name)) {
	TileMapManagerUPtr = std::make_unique<Tiles::TileMapManager>();
}

bool Level::Deserialize(std::istream& iStream, const AssetHeader& header, Level*& out_Level) {
	auto levelUPTR = std::make_unique<Level>("");
	levelUPTR->Name = Serialization::DeserializeStdString(iStream);
	size_t tileMapCount = 0; Serialization::readFromStream(iStream, tileMapCount);
	for (auto i = 0; i < tileMapCount; ++i) {
		Tiles::TileMap* tileMap = nullptr;
		if (!Tiles::TileMap::Deserialize(iStream, tileMap)) {
			std::cout << "Unable to deserialize tileMap index: " << i << " for level: " << levelUPTR->Name << std::endl;
		}
		levelUPTR->TileMapManagerUPtr->tileMaps.push_back(tileMap);
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

bool Level::CanSave(std::string& out_errorMsg, bool allowOverwrite) const {
	if (Name == "untitled") {
		out_errorMsg = "Enter a better name for your level.";
		return false;
	}

	return PersistentAsset<Level>::CanSave(out_errorMsg, allowOverwrite);
}

void Level::Serialize(std::ostream& oStream) const {
	Serialization::Serialize(oStream, Name);
	size_t tileMapCount = TileMapManagerUPtr->tileMaps.size();
	Serialization::writeToStream(oStream, tileMapCount);
	for (const auto& tileMap : TileMapManagerUPtr->tileMaps) {
		tileMap->Serialize(oStream);
	}
}
