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

	class TileMap : public Rendering::Renderable {
		std::unordered_map<glm::ivec2, TileInstance> data;
	public:
		Rendering::Shader* shader = nullptr;

		void SetTile(const Tile* tile, glm::ivec2 grid_position);

		void RemoveTile(glm::ivec2 grid_position) {
			data.erase(grid_position);
		}

		bool TryGetTile(glm::ivec2 grid_position, TileInstance*& out_tileInstance) {
			const auto iterator = data.find(grid_position);
			if(iterator != data.end()) {
				out_tileInstance = &iterator->second;
				return true;
			}
			out_tileInstance = nullptr;
			return false;
		}

		bool TryGetTile(const glm::ivec2 grid_position, const TileInstance*& out_tileInstance) const {
			const auto iterator = data.find(grid_position);
			if(iterator != data.end()) {
				out_tileInstance = &iterator->second;
				return true;
			}
			out_tileInstance = nullptr;
			return false;
		}

		void Render() const override;
	};

}


