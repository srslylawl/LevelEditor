#include "Resources.h"
#include "Texture.h"
#include "Tile.h"
#include "Files.h"


void Resources::LoadTexture(const std::string& path, const bool refresh) {
	using namespace Rendering;
	const auto texIterator = Textures.find(path);
	if (const bool exists = texIterator != Textures.end()) {
		if (refresh) texIterator->second->Refresh();
		return;
	}

	Texture* t = nullptr;
	if(!Texture::Create(path, t)) return;

	Textures[path] = t;
}

void Resources::LoadTile(std::string path, bool refresh) {
	using namespace Tiles;
	const auto tileIterator = Tiles.find(path);
	if (const bool exists = Tiles.count(path)) {
		if (refresh) {} //TODO: implement refresh
		return;
	}

	if(Tile* t = Files::LoadFromFile<Tile>(path.c_str()); t != nullptr) {
		Tiles[path] = t;
	}
	
}

template<typename PointerCollection>
void FreeCollection(PointerCollection pCollection) {
	for (auto p : pCollection)
		delete p;
}

template<typename PointerMap>
void FreeMap(PointerMap pMap) {
	for (auto pair : pMap)
		delete pair.second;
}


void Resources::FreeAll() {
	FreeCollection(Meshes);
	for (auto pair : Textures)
		delete pair.second;

	for (auto pair : Tiles)
		delete pair.second;

}

