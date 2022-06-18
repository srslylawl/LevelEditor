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
#include "Resources.h"
#include "Shader.h"

namespace Tiles {


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

		void Render() const override  {
			// TODO: GPU instancing and pack textures in atlas to render in one draw call as this is highly inefficient
			for (const auto& pair : data) {
				// offset by 0.5f in every axis
				auto offset = glm::vec3(floor(pair.first.x) + 0.5f, floor(pair.first.y) + 0.5f, 0.01);
				glm::mat4 modelM = translate(glm::mat4(1.0f), offset);
				shader->setMat4("model", modelM);
				auto tex = Resources::Textures[pair.second->Texture];
				if (tex.ID == 0) {
					std::cout << "TEXTURE invalid: " << glm::to_string(pair.first) << std::endl;
					continue;
				}
				glBindTexture(GL_TEXTURE_2D, tex.ID);
				Resources::Meshes[1].Draw(); // quad
			}
		}
	};

}


