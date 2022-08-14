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

	if (!Rendering::Texture::LoadFromFile(relative_path, out_texture)) return false;

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

void Resources::AssignOwnership(Rendering::Texture* texture) {
	auto& map = texture->IsInternal() ? InternalTextures : Textures;
	map[texture->IsInternal() ? texture->GetImageFilePath() : texture->AssetId] = texture;
	// Internal textures are referenced by their path

	AssetsIdReferences[texture->AssetId.ToString()] = texture->relativeFilePath;
}

void Resources::AssignOwnership(TextureSheet* sheet) {
	TextureSheets[sheet->AssetId] = sheet;
	AssetsIdReferences[sheet->AssetId] = sheet->GetRelativePath();
}

void Resources::AssignOwnership(Mesh::StaticMesh* mesh) {
	Meshes.push_back(mesh);
}

bool Resources::TryLoadAssetFromHeader(const AssetHeader& header, bool refresh) {
	const auto relPath = header.relativeAssetPath.string();
	switch (header.aType) {
		case AssetType::TextureInternal:
			return LoadInternalTexture(relPath.c_str(), refresh);
		case AssetType::Texture:
			return LoadTexture(relPath.c_str(), refresh);
		case AssetType::TextureSheet:
			return LoadTextureSheet(relPath.c_str(), refresh);
		case AssetType::Tile:
			return LoadTile(relPath.c_str(), refresh);
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
			continue;
		}
		if (!entry.is_regular_file() || entry.is_symlink()) continue;
		AssetHeader aHeader;
		if (AssetHeader::TryReadHeaderFromFile(entry.path(), &aHeader)) {
			if (aHeader.HasCorrespondingFile()) {
				metaFiles.push_back(aHeader);
				continue;
			}

			// Load asset if its a raw file
			TryLoadAssetFromHeader(aHeader, refresh);
			continue;
		}

		nonMetaFiles.insert(Files::GetRelativePath(entry.path()));
	}

	// try to match meta files with their respective files and add to out if successful
	for (auto it = metaFiles.begin(); it != metaFiles.end(); ++it) {
		auto& entry = it;
		bool loaded = AssetIsLoaded(entry->aId);
		if (!loaded && !TryLoadAssetFromHeader(*entry, refresh)) {
			std::cout << "Unable to find corresponding file: " << entry->relativeAssetPath.string().c_str() << " delete meta file if no longer needed" << std::endl;
			continue;
		}

		auto res = nonMetaFiles.find(entry->GetCorrespondingFilePath());
		if (res == nonMetaFiles.end()) {
			std::cerr << "Asset is loaded, but unable to get correct nonMetaFile" << std::endl;
			continue;
		}
		nonMetaFiles.erase(res);
		if (out_Assets != nullptr) out_Assets->push_back(*entry);
	}

	if (nonMetaFiles.empty()) return;
	// TODO: handle unused meta files?

	const std::filesystem::path directoryPath = directory;
	const bool isSpriteSubFolder = Files::PathIsSubPathOfRel(directoryPath, Strings::Directory_Sprites);
	const bool isInternalSpriteSubFolder = Files::PathIsSubPathOfRel(directoryPath, Strings::Directory_Resources_Icons);
	const bool isTextureSheetFolder = Files::PathIsSubPathOfRel(directoryPath, Strings::Directory_TextureSheets);
	const bool isRelevantFolder = isSpriteSubFolder || isInternalSpriteSubFolder || isTextureSheetFolder;
	// now check the remaining non-meta assets and create a meta file if necessary
	for (auto it = nonMetaFiles.begin(); it != nonMetaFiles.end(); ++it) {
		// only create a meta file for supported image
		if (!Rendering::Texture::CanCreateFromPath(it->c_str())) continue;
		// create a texture meta file depending on which folder its in
		if (!isRelevantFolder) continue;
		// create texture meta file
		Rendering::Texture* tex = nullptr;
		AssetHeader header;
		if (!Rendering::Texture::CreateNew(*it, isInternalSpriteSubFolder, isTextureSheetFolder, tex, header)) {
			std::cout << "ERROR: Unable to create texture from path: " << *it << std::endl;
			continue;
		}
		if (isTextureSheetFolder) TextureSheet::CreateNew(tex, header);

		//Debug
#if _DEBUG
		////dbg
		//AssetHeader TESTHEADER;
		auto fullpath = Files::GetAbsolutePath(header.relativeAssetPath.string());
		if (isTextureSheetFolder) {
			TextureSheet* texsheet;
			if (!TextureSheet::LoadFromFile(header.relativeAssetPath.string().c_str(), texsheet)) {
				throw std::exception("Unable to load recently created texturesheet");
			}
		}

		if (isInternalSpriteSubFolder || isSpriteSubFolder) {
			Rendering::Texture* t = nullptr;
			if (!Rendering::Texture::LoadFromFile(header.relativeAssetPath.string().c_str(), t)) {
				throw std::exception("Unable to load recently created texture file");
			}
		}
		////enddbg
#endif
		if (out_Assets != nullptr) {
			out_Assets->push_back(header);
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
	AssignOwnership(t);

	return true;
}

void Resources::LoadAsset(const char* relativePath, bool refresh) {
	AssetHeader aHeader;
	if (!AssetHeader::TryReadHeaderFromFile(Files::GetAbsolutePath(relativePath), &aHeader)) {
		std::cout << "Unable to load asset: " << relativePath << std::endl;
		return;
	}

	if (aHeader.aId.IsEmpty()) {
		std::cout << "Asset guid empty: " << relativePath << std::endl;
	}

	if (!refresh && AssetIsLoaded(aHeader.aId)) return;

	if (!TryLoadAssetFromHeader(aHeader, refresh)) {
		std::cout << "Unable to load asset: " << relativePath << std::endl;
		return;
	}
	AssetsIdReferences[aHeader.aId.ToString()] = relativePath;
}

bool Resources::AssetIsLoaded(const AssetId& id) {
	return AssetsIdReferences.find(id.ToString()) != AssetsIdReferences.end();
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

inline bool TryGetTex(const std::string& key, Rendering::Texture*& out_texture, std::map<std::string, Rendering::Texture*>& map) {
	out_texture = Rendering::Texture::Empty();
	if (const auto it = map.find(key); it != map.end()) out_texture = it->second;
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

