#include "Resources.h"

#include <iostream>
#include <unordered_set>

#include "Texture.h"
#include "Tile.h"
#include "Files.h"
#include "Mesh.h"
#include "TextureSheet.h"


inline bool LoadTex(const char* relative_path, const bool refresh, std::map<std::string, Rendering::Texture*>& map, Rendering::Texture*& out_texture, bool isInternal) {
	out_texture = nullptr;

	const auto texIterator = map.find(relative_path);
	if (const bool exists = texIterator != map.end()) {
		if (refresh) texIterator->second->Refresh();
		out_texture = texIterator->second;
		return true;
	}

	if (!Rendering::Texture::CreateNew(relative_path, out_texture, isInternal)) return false;

	return true;
}

bool Resources::LoadTexture(const char* relative_path, const bool refresh) {
	Rendering::Texture* _;
	return LoadTex(relative_path, refresh, Textures, _, false);
}

bool Resources::LoadTexture(const char* relative_path, Rendering::Texture*& out_texture, const bool refresh) {
	return LoadTex(relative_path, refresh, Textures, out_texture, false);
}


bool Resources::LoadInternalTexture(const char* relative_path, const bool refresh) {
	Rendering::Texture* _;
	return LoadTex(relative_path, refresh, InternalTextures, _, true);
}

void Resources::AddTexture(Rendering::Texture* texture, bool isInternal) {
	auto& map = isInternal ? InternalTextures : Textures;
	map[isInternal ? texture->GetRelativePath() : texture->AssetId] = texture;
}

bool TryLoadAssetFromHeader(const AssetHeader& header, bool refresh) {
	const auto relativePath = Files::GetRelativePath(header.aPath);
	switch (header.aType) {
		case AssetType::TextureInternal:
			return Resources::LoadInternalTexture(relativePath.c_str(), refresh);
		case AssetType::Texture:
			return Resources::LoadTexture(relativePath.c_str(), refresh);
		case AssetType::TextureSheet:
			return Resources::LoadTextureSheet(relativePath.c_str(), refresh);
		case AssetType::Tile:
			return Resources::LoadTile(relativePath.c_str(), refresh);
		case AssetType::Level:
		case AssetType::UNKNOWN:
		default: throw std::exception("unexpected case reached");
	}
}

void Resources::LoadDirectory(const char* directory, bool refresh, bool includeSubdirectories, std::vector<AssetHeader>* out_Assets) {
	auto dir_iterator = Files::GetDirectoryIterator(directory);

	// we want to match each file with their corresponding .asset meta file
	// first pass will sort items into either a collection of meta files or non meta file
	std::vector<AssetHeader> metaFiles;
	std::unordered_set<std::string> nonMetaFiles;

	for (auto& entry : dir_iterator) {
		if (includeSubdirectories && entry.is_directory()) {
			LoadDirectory(entry.path().string().c_str(), refresh, includeSubdirectories, out_Assets);
		}
		AssetHeader aHeader;
		if (AssetHeader::ReadFromFile(entry.path(), &aHeader)) {
			if (aHeader.HasCorrespondingFile()) {
				metaFiles.push_back(aHeader);
			}
			continue;
		}

		nonMetaFiles.insert(entry.path().string());
	}

	// try to match meta files with their respective files and add to out if successful
	for (auto it = metaFiles.begin(); it != metaFiles.end();) {
		auto& entry = it;
		if (TryLoadAssetFromHeader(*it, refresh)) {
			nonMetaFiles.erase(entry->aPath.string());
			if (out_Assets != nullptr) out_Assets->push_back(*entry);
			continue;
		}
		//did not find corresponding asset file, display warning
		std::cout << "Unable to find corresponding file: " << entry->aPath.c_str() << " delete meta file if no longer needed" << std::endl;
		++it;
	}

	// TODO: metaFiles now contains only unused meta files. delete automatically?

	// now check the remaining non-meta assets and create a meta file if necessary
	for (auto it = nonMetaFiles.begin(); it != nonMetaFiles.end(); ++it) {

		// only create a meta file for supported image
		if (!Rendering::Texture::CanCreateFromPath(it->c_str())) continue;

		// create a texture meta file depending on which folder its in
		auto spriteFolderPath = Files::GetAbsolutePath(Strings::Directory_Sprites);
		bool isSpriteSubFolder = Files::PathIsSubPathOf(*it, spriteFolderPath);
		auto internalSpriteFolderPath = Files::GetAbsolutePath(Strings::Directory_Icon);
		bool isInternalSpriteSubFolder = Files::PathIsSubPathOf(*it, internalSpriteFolderPath);
		if (isSpriteSubFolder || isInternalSpriteSubFolder) {
			// create texture meta file
			Rendering::Texture* tex = nullptr;
			if (Rendering::Texture::CreateNew(*it, tex, isInternalSpriteSubFolder)) {
				tex->SaveToFile();
				if (out_Assets != nullptr) {
					AssetHeader header;
					if (!AssetHeader::ReadFromFile(Files::GetAbsolutePath(tex->GetRelativePath()), &header)) {
						std::cout << "Header does not exist after creating texture!" << std::endl;
					}
					out_Assets->push_back(header);
				}
			}
			continue;
		}

		auto textureSheetFolderPath = Files::GetAbsolutePath(Strings::Directory_TextureSheets);
		if (Files::PathIsSubPathOf(*it, textureSheetFolderPath)) {
			// create texturesheet
			std::string relativePathStr = Files::GetRelativePath(*it);
			Rendering::Texture* loadedTexture = nullptr;
			if (!LoadTexture(relativePathStr.c_str(), loadedTexture, refresh)) continue;

			auto relativePath = Files::GetRelativePath(*it);
			auto sheet = TextureSheet::CreateNew(loadedTexture);
			sheet->SaveToFile();
			auto path = sheet->GetRelativePath();
			delete sheet;
			LoadTextureSheet(path.c_str());
			if (out_Assets != nullptr) {
				AssetHeader header;
				if (!AssetHeader::ReadFromFile(Files::GetAbsolutePath(path), &header)) {
					std::cout << "Header does not exist after creating textureSheet!" << std::endl;
				}
				out_Assets->push_back(header);
			}
		}
	}
}

bool Resources::LoadTextureSheet(const char* relative_path, bool refresh) {
	const auto tsIt = TextureSheets.find(relative_path);
	if (tsIt != TextureSheets.end()) {
		if (refresh) {
			throw std::exception("texture sheet refresh not implemented");
		} //TODO: implement refresh
		return true;
	}

	TextureSheet* t = nullptr;
	if (!TextureSheet::LoadFromFile(relative_path, t)) {
		std::cout << "Unable to load Texturesheet: " << relative_path << std::endl;
		return false;
	}
	TextureSheets[t->AssetId] = t;

	return true;
}

bool Resources::LoadTile(const char* relative_path, bool refresh) {
	using namespace Tiles;
	const auto tileIterator = Tiles.find(relative_path);
	if (tileIterator != Tiles.end()) {
		if (refresh) {
			throw std::exception("tile refresh not implemented");
		} //TODO: implement refresh
		return true;
	}

	Tile* t = nullptr;
	if (!Tile::LoadFromFile(relative_path, t)) {
		std::cout << "unable to load Tile: " << relative_path << std::endl;
		return false;
	}

	Tiles[t->AssetId] = t;
	return true;
}

inline bool TryGetTex(const std::string& assetId, Rendering::Texture*& out_texture, std::map<std::string, Rendering::Texture*>& map) {
	out_texture = Rendering::Texture::Empty();
	if (const auto it = map.find(assetId); it != map.end()) out_texture = it->second;
	return out_texture != Rendering::Texture::Empty();
}

//Returns Texture::Empty() on fail.
bool Resources::TryGetTexture(const std::string& assetId, Rendering::Texture*& out_texture) {
	return TryGetTex(assetId, out_texture, Textures);
}

unsigned Resources::TryGetTextureId(const std::string& assetId) {
	unsigned int textureId = 0;
	if (Rendering::Texture* t; TryGetTexture(assetId, t)) {
		textureId = t->GetTextureID();
	}
	return textureId;
}

bool Resources::TryGetInternalTexture(const char* relativePath, Rendering::Texture*& out_texture) {
	return TryGetTex(relativePath, out_texture, InternalTextures);
}

bool Resources::TryGetTile(const std::string& assetId, Tiles::Tile*& out_tile) {
	out_tile = nullptr;
	if (const auto it = Tiles.find(assetId); it != Tiles.end()) out_tile = it->second;

	return out_tile != nullptr;
}

bool Resources::TryGetTextureSheet(const std::string& assetId, TextureSheet*& out_textureSheet) {
	out_textureSheet = nullptr;
	if (const auto it = TextureSheets.find(assetId); it != TextureSheets.end()) out_textureSheet = it->second;

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

