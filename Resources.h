#pragma once
#include <map>

#include "Files.h"
#include "Mesh.h"
#include "Texture.h"


class Resources {
public:
	inline static std::map<std::string, Texture> Textures;
	static void LoadTexture(std::string path, bool refresh = false);

	inline static std::vector<Mesh::StaticMesh> Meshes;

	static void FreeAll() {
		for ( auto& mesh : Meshes) {
			mesh.UnloadFromGPU();
		}
	}
};

