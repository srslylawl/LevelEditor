#pragma once
#include <map>
#include <ostream>
#include <string>
#include <glm/vec2.hpp>
#include <utility>
#include <glm/fwd.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glm/gtx/hash.hpp"

#include "Renderable.h"

namespace Rendering {
	class Shader;
}

namespace Tiles {
	class Tile;

	class TileMap : public Rendering::Renderable {
		std::unordered_map<glm::ivec2, const Tile*> data;
	public:
		Rendering::Shader* shader;

		void SetTile(const Tile* tile, glm::ivec2 grid_position) {
			data[grid_position] = tile;
		}

		void RemoveTile(glm::ivec2 grid_position) {
			data.erase(grid_position);
		}

		void Render() const override;
	};

}


