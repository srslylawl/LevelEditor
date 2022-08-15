#pragma once
#include <map>
#include <string>
#include <vector>

#include "Assets.h"


namespace Rendering {
	class Texture;
	class TextureSheet;
}

namespace Tiles {
	class Tile;
}


namespace Mesh {
	class StaticMesh;
}

class Resources {
	inline static std::map<std::string, std::string> AssetsIdReferences;
	inline static std::map<std::string, Rendering::Texture*> Textures;
	inline static std::map<std::string, Rendering::Texture*> InternalTextures;
	inline static std::map<std::string, Tiles::Tile*> Tiles;
	inline static std::map<std::string, Rendering::TextureSheet*> TextureSheets;
	inline static std::vector<Mesh::StaticMesh*> Meshes;

	static bool LoadTile(const char* relative_path, bool refresh = false);
	static bool LoadTexture(const char* relative_path, bool refresh = false);
	static bool LoadTexture(const char* relative_path, Rendering::Texture*& out_texture, bool refresh = false);
	static bool LoadTextureSheet(const char* relative_path, bool refresh = false);
	static bool LoadInternalTexture(const char* relative_path, bool refresh = false);
	static bool TryLoadAssetFromHeader(const AssetHeader& header, bool refresh);
public:

	static const std::map<std::string, Rendering::Texture*>& GetInternalTextures() {
		return InternalTextures;
	}

	static void LoadAsset(const char* relativePath, bool refresh = false);
	static bool AssetIsLoaded(const AssetId& id);

	static void LoadDirectory(const char* directory, bool refresh, bool includeSubdirectories,
	                          std::vector<AssetHeader>* out_Assets = nullptr);
	static void AssignOwnership(Rendering::TextureSheet* sheet);
	static void AssignOwnership(Rendering::Texture* texture);
	static void AssignOwnership(Mesh::StaticMesh* mesh);


	static bool TryGetTexture(const std::string& assetId, Rendering::Texture*& out_texture);
	static bool TryGetInternalTexture(const char* relativePath, Rendering::Texture*& out_texture);
	static bool TryGetTile(const std::string& assetId, Tiles::Tile*& out_tile);
	static bool TryGetTextureSheet(const std::string& assetId, Rendering::TextureSheet*& out_textureSheet);
	static unsigned TryGetTextureId(const std::string& assetId);

	static void FreeAll();
};