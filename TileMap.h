#pragma once
#include <glm/vec2.hpp>
#include <glm/fwd.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glm/gtx/hash.hpp"

#include "Renderable.h"
#include "TileMap.h"
#include "TileInstance.h"
#include <map>

#include "Assets.h"

namespace Rendering {
	class Shader;
}

namespace Tiles {
	class Tile;


	//TODO: display a warning/hint when selecting a tile that does not match tilemap type?
	enum class TileMapType {
		Any,
		Floor,
		Wall,
		Ceiling
	};

	class TileMap : public Rendering::Renderable, public Serialization::Serializable<TileMap> {
		std::unordered_map<glm::ivec2, TileInstance> data{};
		std::map<std::string, int> tileReferences{};

		void RefreshSurroundingTileInstances(const glm::ivec2 position);
		void ReduceTileReferences(const Tile* tile);
	public:
		void SetTile(const Tile* tile, glm::ivec2 grid_position);
		void RemoveTile(glm::ivec2 grid_position);
		bool TryGetTile(glm::ivec2 grid_position, TileInstance*& out_tileInstance);
		bool TryGetTile(const glm::ivec2 grid_position, TileInstance*& out_tileInstance) const;

		glm::ivec2 ConvertToTileMapGridPosition(glm::ivec2 grid_position) const;

		TileMapType Type;
		glm::ivec2 GridDimensions;
		glm::ivec2 TileDimensions;

		TileMap() : TileMap("") { }
		explicit TileMap(std::string name, TileMapType type = TileMapType::Any, glm::ivec2 tileDimensions = glm::ivec2(1,1), glm::ivec2 gridDimensions = glm::ivec2(1,1)) : Serializable(name), Type(type), GridDimensions(gridDimensions), TileDimensions(tileDimensions) { }

		void Render() const override;

		void RenderImGui();

		static bool Deserialize(std::istream& iStream, TileMap*& out_tileMap);
		void Serialize(std::ostream& oStream) const override;
	};

}


