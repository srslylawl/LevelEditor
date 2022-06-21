#pragma once
#include <map>
#include <string>
#include <vector>


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
public:
	inline static std::map<std::string, Rendering::Texture*> Textures;
	inline static std::map<std::string, Tiles::Tile*> Tiles;
	inline static std::vector<Mesh::StaticMesh*> Meshes;

	static void LoadTexture(const std::string& path, bool refresh = false);
	static void LoadTile(const std::string& path, bool refresh = false);
	static bool TryGetTexture(const std::string& path, Rendering::Texture*& out_texture);

	static void FreeAll();
};

