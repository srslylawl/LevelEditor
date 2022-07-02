#include "Resources.h"

#include <iostream>

#include "Texture.h"
#include "Tile.h"
#include "Files.h"
#include "Mesh.h"


inline void LoadTex(const char* relative_path, const bool refresh, std::map<std::string, Rendering::Texture*>& map) {
	const auto texIterator = map.find(relative_path);
	if (const bool exists = texIterator != map.end()) {
		if (refresh) texIterator->second->Refresh();
		return;
	}

	Rendering::Texture* t = nullptr;
	if (!Rendering::Texture::Create(relative_path, t)) return;

	map[relative_path] = t;
}
void Resources::LoadTexture(const char* relative_path, const bool refresh) {
	LoadTex(relative_path, refresh, Textures);
}

void Resources::LoadInternalTexture(const char* relative_path, const bool refresh) {
	LoadTex(relative_path, refresh, InternalTextures);
}

bool Resources::LoadTile(const char* relative_path, bool refresh) {
	using namespace Tiles;
	const auto tileIterator = Tiles.find(relative_path);
	if (tileIterator != Tiles.end()) {
		if (refresh) {} //TODO: implement refresh
		return true;
	}

	Tile* t = nullptr;
	if (!Files::LoadFromFile(relative_path, t)) {
		std::cout << "unable to load Tile: " << relative_path << std::endl;
		return false;
	}

	Tiles[relative_path] = t;
	return true;
}

inline bool TryGetTex(const char* relative_path, Rendering::Texture*& out_texture, std::map<std::string, Rendering::Texture*>& map) {
	out_texture = nullptr;
	if (const auto it = map.find(relative_path); it != map.end()) out_texture = it->second;
	return out_texture != nullptr;
}

bool Resources::TryGetTexture(const char* relative_path, Rendering::Texture*& out_texture) {
	return TryGetTex(relative_path, out_texture, Textures);
}

bool Resources::TryGetInternalTexture(const char* relative_path, Rendering::Texture*& out_texture) {
	return TryGetTex(relative_path, out_texture, InternalTextures);
}

bool Resources::TryGetTile(const char* relative_path, Tiles::Tile*& out_tile) {
	out_tile = nullptr;
	if(const auto it = Tiles.find(relative_path); it != Tiles.end()) out_tile = it->second;

	return out_tile != nullptr;
}

template<typename PointerCollection>
void FreeCollection(PointerCollection pCollection) {
	for (auto& p : pCollection)
		delete p;
}

template<typename PointerMap>
void FreeMap(PointerMap pMap) {
	for (auto& pair : pMap)
		delete pair.second;
}


void Resources::FreeAll() {
	FreeCollection(Meshes);
	for (auto& pair : Textures)
		delete pair.second;

	for (auto& pair : Tiles)
		delete pair.second;

}

