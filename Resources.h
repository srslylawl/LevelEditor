#pragma once
#include <map>
#include <string>
#include <vector>


class TextureSheet;

namespace Tiles {
	class Tile;
}
namespace Rendering {
	class Texture;
}

namespace Mesh {
	class StaticMesh;
}

class Resources {
	inline static std::map<std::string, Rendering::Texture*> Textures;
	inline static std::map<std::string, Rendering::Texture*> InternalTextures;
public:
	inline static std::map<std::string, Tiles::Tile*> Tiles;
	inline static std::map<std::string, TextureSheet*> TextureSheets;
	inline static std::vector<Mesh::StaticMesh*> Meshes;

	static const std::map<std::string, Rendering::Texture*>& GetTextures() {
		const std::map<std::string, Rendering::Texture*>& t = Textures;
		return t;
	}

	static const std::map<std::string, Rendering::Texture*>& GetInternalTextures() {
		return InternalTextures;
	}

	static void LoadTexture(const char* relative_path, bool refresh = false);
	static bool LoadTexture(const char* relative_path, Rendering::Texture*& out_texture, bool refresh = false);

	static void AddTexture(Rendering::Texture* texture, bool isInternal = false);

	static void LoadInternalTexture(const char* relative_path, bool refresh = false);
	static void HandleTextureSheetFolder(bool refresh = false);
	static void LoadTextureSheet(const char* relative_path, bool refresh = false);

	static bool LoadTile(const char* relative_path, bool refresh = false);

	static bool TryGetTexture(const char* relative_path, Rendering::Texture*& out_texture);
	static unsigned TryGetTextureId(const char* relative_path);
	static bool TryGetInternalTexture(const char* relative_path, Rendering::Texture*& out_texture);
	static bool TryGetTile(const char* relative_path, Tiles::Tile*& out_tile, bool tryLoad = false);
	static bool TryGetTextureSheet(const char* relative_path, TextureSheet*& out_textureSheet);

	static void FreeAll();
};