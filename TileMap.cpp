#include "TileMap.h"
#include "Mesh.h"
#include "Shader.h"
#include "TileInstance.h"
#include "Renderer.h"
#include "Files.h"
#include "Resources.h"
#include "Tile.h"

void Tiles::TileMap::RefreshSurroundingTileInstances(const glm::ivec2 position) {
	TileInstance* t = nullptr;
	for (int x = -1; x <= 1; ++x) {
		for (int y = -1; y <= 1; ++y) {
			if (x == 0 && y == 0) continue; //skip self
			const auto tilePos = glm::ivec2(position.x + x, position.y + y);
			if (TryGetTile(tilePos, t))t->Refresh(tilePos, this);
		}
	}
}

void Tiles::TileMap::ReduceTileReferences(const Tile* tile) {
	auto path = tile->GetRelativePath();
	--tileReferences[path];

	if (tileReferences[path] == 0) {
		tileReferences.erase(path);
	}
}

void Tiles::TileMap::SetTile(const Tile* tile, glm::ivec2 grid_position) {
	grid_position = ConvertToTileMapGridPosition(grid_position);
	auto ti = TileInstance(tile, grid_position, this);
	if (auto it = data.find(grid_position); it != data.end()) {
		if (it->second.GetParent() != tile) {
			//being replaced with a different tile
			ReduceTileReferences(it->second.GetParent());
		}
		it->second = ti;
	}
	else {
		//New tile at this position
		data.emplace(grid_position, ti);
		++tileReferences[tile->GetRelativePath()];
	}


	RefreshSurroundingTileInstances(grid_position);
}

void Tiles::TileMap::RemoveTile(glm::ivec2 grid_position) {
	grid_position = ConvertToTileMapGridPosition(grid_position);
	auto it = data.find(grid_position);
	if (it == data.end()) {
		return;
	}
	ReduceTileReferences(it->second.GetParent());
	data.erase(grid_position);
	RefreshSurroundingTileInstances(grid_position);
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

bool Tiles::TileMap::TryGetTile(const glm::ivec2 grid_position, TileInstance*& out_tileInstance) const {
	const auto gridPos = ConvertToTileMapGridPosition(grid_position);
	auto iterator = data.find(gridPos);
	if (iterator != data.end()) {
		out_tileInstance = const_cast<TileInstance*>(&iterator->second);
		return true;
	}
	out_tileInstance = nullptr;
	return false;
}

glm::ivec2 Tiles::TileMap::ConvertToTileMapGridPosition(glm::ivec2 grid_position) const {
	if (GridDimensions == glm::ivec2(1, 1)) return grid_position;

	// Not 100% sure why, but since the grid goes from 1 to 0 to -1 we have to offset it by 1-dimension in order to be properly aligned
	const int minusOffsetX = grid_position.x < 0 ? 1 - GridDimensions.x : 0;
	const int minusOffsetY = grid_position.y < 0 ? 1 - GridDimensions.y : 0;
	grid_position.x += minusOffsetX;
	grid_position.y += minusOffsetY;
	int offsetX = abs(grid_position.x) % GridDimensions.x;
	int offsetY = abs(grid_position.y) % GridDimensions.y;

	if (grid_position.x < 0) {
		offsetX *= -1;
	}
	if (grid_position.y < 0) {
		offsetY *= -1;
	}
	const glm::ivec2 offset = glm::vec2(offsetX, offsetY);

	return grid_position - offset;
}

Tiles::TileMap::TileMap(std::string name, TileMapType type, glm::ivec2 gridDimensions) : Type(type), GridDimensions(gridDimensions) {
	Name = name;
}

void Tiles::TileMap::Render() const {
	using namespace glm;
	// TODO: GPU instancing and pack textures in atlas to render in one draw call as this is highly inefficient
	for (const auto& pair : data) {
		// offset by half in every axis
		auto offset = vec3(floor(pair.first.x) + GridDimensions.x / 2.0f, floor(pair.first.y) + GridDimensions.y / 2.0f, 0.01);

		mat4 modelM = translate(mat4(1.0f), offset);
		if (GridDimensions != ivec2(1, 1)) {
			modelM = scale(modelM, vec3(GridDimensions, 1));
		}
		Rendering::Renderer::defaultShader->setMat4("model", modelM);

		glBindTexture(GL_TEXTURE_2D, pair.second.GetActiveTextureId());
		Mesh::StaticMesh::GetDefaultQuad()->Draw();
	}
}

bool Tiles::TileMap::Deserialize(std::istream& iStream, TileMap*& out_tileMap) {
	auto tileMapUPTR = std::make_unique<Tiles::TileMap>();
	tileMapUPTR->Name = Serialization::DeserializeStdString(iStream);
	Serialization::readFromStream(iStream, tileMapUPTR->Type);
	Serialization::readFromStream(iStream, tileMapUPTR->GridDimensions.x);
	Serialization::readFromStream(iStream, tileMapUPTR->GridDimensions.y);

	size_t tileCount = 0; Serialization::readFromStream(iStream, tileCount);
	if (tileCount > 0) {
		//TileIndexTable
		size_t tileRefSize = 0; Serialization::readFromStream(iStream, tileRefSize);
		std::map<int, const Tile*> tileIndexTable;
		for (auto i = 0; i < tileRefSize; ++i) {
			::AssetId tileId; Serialization::TryDeserializeAssetId(iStream, tileId);
			int tileIndex = 0; Serialization::readFromStream(iStream, tileIndex);
			Tiles::Tile* t = nullptr;
			if (!Resources::TryGetTile(tileId, t)) {
				std::string msg = "unable to load tile " + tileId.ToString();
				throw std::exception(msg.c_str());
			}
			tileIndexTable[tileIndex] = t;
		}

		//TileInfo
		for (int i = 0; i < tileCount; ++i) {
			glm::ivec2 position(0, 0);
			Serialization::readFromStream(iStream, position.x);
			Serialization::readFromStream(iStream, position.y);
			int tileIndex = 0; Serialization::readFromStream(iStream, tileIndex);
			Tiles::SurroundingTileFlags mask = SurroundingTileFlags::NONE; Serialization::readFromStream(iStream, mask);
			const Tile* tile = tileIndexTable[tileIndex];
			tileMapUPTR->data.emplace(position, TileInstance(tile, mask, position));
			++tileMapUPTR->tileReferences[tile->GetRelativePath()];
		}
	}

	std::string tileMapStringVerification("tileMap");
	std::string isVerified = Serialization::DeserializeStdString(iStream);
	if (isVerified != tileMapStringVerification) {
		std::cout << "Failed to deserialize tilemap, string verification failed" << std::endl;
		return false;
	}

	out_tileMap = tileMapUPTR.release();
	return true;
}

void Tiles::TileMap::Serialize(std::ostream& oStream) const {
	Serialization::Serialize(oStream, Name);
	/*int type = (int)Type; */Serialization::writeToStream(oStream, Type);
	Serialization::writeToStream(oStream, GridDimensions.x);
	Serialization::writeToStream(oStream, GridDimensions.y);

	auto tileCount = data.size();
	Serialization::writeToStream(oStream, tileCount);
	if (tileCount > 0) {
		//Build tile index table
		auto tileRefCount = tileReferences.size();
		if (tileRefCount < 1) throw std::exception("tile reference count is 0 when tile count is > 0");
		Serialization::writeToStream(oStream, tileRefCount);
		std::map<const Tile*, int> tileIndexTable;

		int refIndex = 0;
		for (auto refIt = tileReferences.begin(); refIt != tileReferences.end(); ++refIt) {
			Tile* t = nullptr;
			if (!Resources::TryGetTile(refIt->first, t)) {
				std::string msg = "unable to serialize tileMap: " + Name + ": tile not found: " + refIt->first;
				throw std::exception(msg.c_str());
			}
			tileIndexTable[t] = refIndex;

			Serialization::Serialize(oStream, refIt->first);
			Serialization::writeToStream(oStream, refIndex);
			++refIndex;
		}

		//Write tile info
		for (const auto& [pos, tileInstance] : data) {
			Serialization::writeToStream(oStream, pos.x);
			Serialization::writeToStream(oStream, pos.y);
			Serialization::writeToStream(oStream, tileIndexTable[tileInstance.GetParent()]);
			Serialization::writeToStream(oStream, tileInstance.GetMask());
		}
	}
	std::string tileMapStringVerification("tileMap");
	Serialization::Serialize(oStream, tileMapStringVerification);
}
