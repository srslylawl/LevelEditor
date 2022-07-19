#include "TileMap.h"
#include "Mesh.h"
#include "Shader.h"
#include "TileInstance.h"

void Tiles::TileMap::SetTile(const Tile* tile, glm::ivec2 grid_position) {
	auto ti = TileInstance(tile, grid_position, this);
	if (auto it = data.find(grid_position); it != data.end())
		it->second = ti;
	else
		data.insert({ grid_position, ti });
	TileInstance::RefreshSurroundingTileInstances(grid_position, this);
}

void Tiles::TileMap::Render() const {
	// TODO: GPU instancing and pack textures in atlas to render in one draw call as this is highly inefficient
	for (const auto& pair : data) {
		// offset by 0.5f in every axis
		auto offset = glm::vec3(floor(pair.first.x) + 0.5f, floor(pair.first.y) + 0.5f, 0.01);

		glm::mat4 modelM = translate(glm::mat4(1.0f), offset);
		shader->setMat4("model", modelM);

		glBindTexture(GL_TEXTURE_2D, pair.second.GetActiveTextureId());
		Mesh::StaticMesh::GetDefaultQuad()->Draw();
	}
}
