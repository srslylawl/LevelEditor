#include "Resources.h"

#include <iostream>

#include "Texture.h"
#include "Tile.h"
#include "Files.h"
#include "Mesh.h"
#include "TextureSheet.h"


inline void LoadTex(const char* relative_path, const bool refresh, std::map<std::string, Rendering::Texture*>& map, Rendering::Texture*& out_texture, bool isInternal) {
	out_texture = nullptr;

	const auto texIterator = map.find(relative_path);
	if (const bool exists = texIterator != map.end()) {
		if (refresh) texIterator->second->Refresh();
		out_texture = texIterator->second;
		return;
	}

	if (!Rendering::Texture::Create(relative_path, out_texture, isInternal)) return;
}
void Resources::LoadTexture(const char* relative_path, const bool refresh) {
	Rendering::Texture* _;
	LoadTex(relative_path, refresh, Textures, _, false);
}

bool Resources::LoadTexture(const char* relative_path, Rendering::Texture*& out_texture, const bool refresh) {
	LoadTex(relative_path, refresh, Textures, out_texture, false);
	return out_texture != nullptr;
}

void Resources::AddTexture(Rendering::Texture* texture, bool isInternal) {
	auto &map = isInternal ? InternalTextures : Textures;
	map[texture->GetRelativeFilePath()] = texture;
}

void Resources::LoadInternalTexture(const char* relative_path, const bool refresh) {
	Rendering::Texture* _;
	LoadTex(relative_path, refresh, InternalTextures, _, true);
}

void Resources::HandleTextureSheetFolder(bool refresh) {
	//TODO: this only handles the root directory currently
	auto it = Files::GetDirectoryIterator(Strings::Directory_TextureSheets);

	//Load all textures first
	for (auto& entry : it) {
		std::string absolutePath = entry.path().string();

		if (!Rendering::Texture::CanCreateFromPath(absolutePath.c_str())) continue;

		std::string relativePathStr = Files::GetRelativePath(absolutePath);
		Rendering::Texture* loadedTexture = nullptr;
		if (!LoadTexture(relativePathStr.c_str(), loadedTexture, refresh)) continue;

		//Check if texture has a corresponding SpriteSheet
		std::filesystem::path relativeTexSheetPath = std::filesystem::path(relativePathStr).replace_extension(TextureSheet::FileEnding);;
		std::string relativeTexSheetPathStr = relativeTexSheetPath.string();

		if (std::filesystem::exists(relativeTexSheetPath)) {
			LoadTextureSheet(relativeTexSheetPathStr.c_str(), refresh);
			continue;
		}

		//Texture does not have a corresponding texture sheet, so we create one!
		TextureSheets[relativeTexSheetPathStr] = new TextureSheet(loadedTexture);
		Files::SaveToFile(TextureSheets[relativeTexSheetPathStr]);
	}

	//Check the remaining TextureSheet files - if one is not loaded yet, that means that its corresponding image file is missing
	for (auto& entry : it) {
		if (entry.path().extension().string() == TextureSheet::FileEnding) {
			auto relPath = Files::GetRelativePath(entry.path());
			if (TextureSheets.find(relPath) == TextureSheets.end()) {
				// not loaded, which means its texture was not found
				std::cout << "TextureSheet " << relPath << " 's corresponding Image file was not found." << std::endl;
			}
		}
	}

}

void Resources::LoadTextureSheet(const char* relative_path, bool refresh) {
	const auto tsIt = TextureSheets.find(relative_path);
	if (tsIt != TextureSheets.end()) {
		if (refresh) {} //TODO: implement refresh
		return;
	}

	TextureSheet* t = nullptr;
	if (!Files::LoadFromFile(relative_path, t)) {
		std::cout << "Unable to load Texturesheet: " << relative_path << std::endl;
		return;
	}
	TextureSheets[relative_path] = t;
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
	out_texture = Rendering::Texture::Empty();
	if (const auto it = map.find(relative_path); it != map.end()) out_texture = it->second;
	return out_texture != Rendering::Texture::Empty();
}

//Returns Texture::Empty() on fail.
bool Resources::TryGetTexture(const char* relative_path, Rendering::Texture*& out_texture) {
	return TryGetTex(relative_path, out_texture, Textures);
}

bool Resources::TryGetInternalTexture(const char* relative_path, Rendering::Texture*& out_texture) {
	return TryGetTex(relative_path, out_texture, InternalTextures);
}

bool Resources::TryGetTile(const char* relative_path, Tiles::Tile*& out_tile) {
	out_tile = nullptr;
	if (const auto it = Tiles.find(relative_path); it != Tiles.end()) out_tile = it->second;

	return out_tile != nullptr;
}

bool Resources::TryGetTextureSheet(const char* relative_path, TextureSheet*& out_textureSheet) {
	out_textureSheet = nullptr;
	if(const auto it = TextureSheets.find(relative_path); it!=TextureSheets.end()) out_textureSheet = it->second;

	return out_textureSheet != nullptr;
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

	for (auto& pair : TextureSheets) {
		delete pair.second;
	}

}

