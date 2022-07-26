#include "TileMap.h"
#include "Mesh.h"
#include "Shader.h"
#include "TileInstance.h"
#include "Renderer.h"

void Tiles::TileMap::SetTile(const Tile* tile, glm::ivec2 grid_position) {
	grid_position = ConvertToTileMapGridPosition(grid_position);
	auto ti = TileInstance(tile, grid_position, this);
	if (auto it = data.find(grid_position); it != data.end())
		it->second = ti;
	else
		data.emplace(grid_position, ti);
	TileInstance::RefreshSurroundingTileInstances(grid_position, this);
}

void Tiles::TileMap::RemoveTile(glm::ivec2 grid_position) {
	grid_position = ConvertToTileMapGridPosition(grid_position);
	data.erase(grid_position);
	TileInstance::RefreshSurroundingTileInstances(grid_position, this);
}

bool Tiles::TileMap::TryGetTile(glm::ivec2 grid_position, TileInstance*& out_tileInstance) {
	grid_position = ConvertToTileMapGridPosition(grid_position);
	const auto iterator = data.find(grid_position);
	if (iterator != data.end()) {
		out_tileInstance = &iterator->second;
		return true;
	}
	out_tileInstance = nullptr;
	return false;
}

bool Tiles::TileMap::TryGetTile(const glm::ivec2 grid_position, const TileInstance*& out_tileInstance) const {
	const auto gridPos = ConvertToTileMapGridPosition(grid_position);
	const auto iterator = data.find(gridPos);
	if (iterator != data.end()) {
		out_tileInstance = &iterator->second;
		return true;
	}
	out_tileInstance = nullptr;
	return false;
}

glm::ivec2 Tiles::TileMap::ConvertToTileMapGridPosition(glm::ivec2 grid_position) const {
	if(GridDimensions == glm::ivec2(1, 1)) return grid_position;

	// Not 100% sure why, but since the grid goes from 1 to 0 to -1 we have to offset it by 1-dimension in order to be properly aligned
	const int minusOffsetX =  grid_position.x < 0 ? 1-GridDimensions.x : 0;
	const int minusOffsetY =  grid_position.y < 0 ? 1-GridDimensions.y : 0;
	grid_position.x += minusOffsetX;
	grid_position.y += minusOffsetY;
	int offsetX = abs(grid_position.x) % GridDimensions.x;
	int offsetY = abs(grid_position.y) % GridDimensions.y;

	if(grid_position.x < 0) {
		offsetX *= -1;
	}
	if(grid_position.y < 0) {
		offsetY *= -1;
	}
	const glm::ivec2 offset = glm::vec2(offsetX, offsetY);

	return grid_position - offset;
}

Tiles::TileMap::TileMap(std::string name, TileMapType type, glm::ivec2 gridDimensions) : Name(name), Type(type), GridDimensions(gridDimensions) {
}

void Tiles::TileMap::Render() const {
	using namespace glm;
	// TODO: GPU instancing and pack textures in atlas to render in one draw call as this is highly inefficient
	for (const auto& pair : data) {
		// offset by half in every axis
		auto offset = vec3(floor(pair.first.x) + GridDimensions.x/2.0f, floor(pair.first.y) + GridDimensions.y/2.0f, 0.01);

		mat4 modelM = translate(mat4(1.0f), offset);
		if (GridDimensions != ivec2(1, 1)) {
			modelM = scale(modelM, vec3(GridDimensions, 1));
		}
		Rendering::Renderer::defaultShader->setMat4("model", modelM);

		glBindTexture(GL_TEXTURE_2D, pair.second.GetActiveTextureId());
		Mesh::StaticMesh::GetDefaultQuad()->Draw();
	}
}
