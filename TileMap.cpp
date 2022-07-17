#include "TileMap.h"
#include "Mesh.h"
#include "Resources.h"
#include "Shader.h"
#include "Tile.h"
#include "Texture.h"

void Tiles::TileMap::Render() const {
	// TODO: GPU instancing and pack textures in atlas to render in one draw call as this is highly inefficient
	for (const auto& pair : data) {
		// offset by 0.5f in every axis
		auto offset = glm::vec3(floor(pair.first.x) + 0.5f, floor(pair.first.y) + 0.5f, 0.01);

		glm::mat4 modelM = translate(glm::mat4(1.0f), offset);
		shader->setMat4("model", modelM);

		Rendering::Texture* tex = nullptr;
		if(!Resources::TryGetTexture(pair.second->DisplayTexture.c_str(), tex)) {
			std::cout << "TEXTURE invalid: " << glm::to_string(pair.first) << std::endl;
			continue;
		}

		glBindTexture(GL_TEXTURE_2D, tex->GetTextureID());
		Mesh::StaticMesh::GetDefaultQuad()->Draw();
	}
}
