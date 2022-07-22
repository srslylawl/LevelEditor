#pragma once
#include <glm/vec2.hpp>
#include <glm/fwd.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glm/gtx/hash.hpp"

#include "Renderable.h"
#include "TileMap.h"
#include "TileInstance.h"

namespace Rendering {
	class Shader;
}

namespace Tiles {
	class Tile;

	enum class TileMapType {
		Any,
		Floor,
		Wall,
		Ceiling
	};

	class TileMap : public Rendering::Renderable {
		std::unordered_map<glm::ivec2, TileInstance> data;
	public:
		Rendering::Shader* shader = nullptr;

		void SetTile(const Tile* tile, glm::ivec2 grid_position);
		void RemoveTile(glm::ivec2 grid_position);
		bool TryGetTile(glm::ivec2 grid_position, TileInstance*& out_tileInstance);
		bool TryGetTile(const glm::ivec2 grid_position, const TileInstance*& out_tileInstance) const;

		std::string Name;
		TileMapType Type;

		TileMap(std::string name, TileMapType type = TileMapType::Any, Rendering::Shader* shader = nullptr);

		void Render() const override;
	};

}


