#pragma once

#include <unordered_map>
#include <utility>
#include <vector>
#include <xhash>
#include "glm/glm.hpp"

struct Vertex {
	Vertex(const float& pos_x, const float& pos_y, const float& pos_z, const float& uv_x, const float& uv_y) : Position(pos_x, pos_y, pos_z), TexCoords(uv_x, uv_y) {}

	glm::vec3 Position;
	glm::vec2 TexCoords;

	friend bool operator==(const Vertex& lhs, const Vertex& rhs) {
		return lhs.Position == rhs.Position
			&& lhs.TexCoords == rhs.TexCoords;
	}


};

namespace std {
	template<>
	struct hash<Vertex> {
		std::size_t operator()(const Vertex& obj) const noexcept {
			std::size_t seed = 0x7B8FFA02;
			seed ^= (seed << 6) + (seed >> 2) + 0x6F91E7B5 + static_cast<std::size_t>(obj.Position.x);
			seed ^= (seed << 6) + (seed >> 2) + 0x0A5A4B04 + static_cast<std::size_t>(obj.Position.y);
			seed ^= (seed << 6) + (seed >> 2) + 0x26D9AAEB + static_cast<std::size_t>(obj.Position.z);
			seed ^= (seed << 6) + (seed >> 2) + 0x2090388E + static_cast<std::size_t>(obj.TexCoords.x);
			seed ^= (seed << 6) + (seed >> 2) + 0x5203E2B2 + static_cast<std::size_t>(obj.TexCoords.y);
			return seed;
		}
	};
}

namespace Mesh {
	inline void PackVertexData(const float* in_vertPos, const float* in_texCoords, int in_vertCount, std::vector<Vertex>& out_vertices) {
		for (int vertex = 0; vertex < in_vertCount; vertex++) {
			const int currVertPos = vertex * 3;
			const int currTexPos = vertex * 2;
			out_vertices.emplace_back(in_vertPos[currVertPos],
				in_vertPos[currVertPos + 1],
				in_vertPos[currVertPos + 2],
				in_texCoords[currTexPos],
				in_texCoords[currTexPos + 1]);
		}
	}

	inline bool GetSimilarVertexIndex(const Vertex& vert, std::unordered_map<Vertex, unsigned int>& VertexToOutIndex, unsigned int& result) {
		auto it = VertexToOutIndex.find(vert);
		if (it == VertexToOutIndex.end()) {
			return false;
		}

		result = it->second;
		return true;
	}

	inline void GenIndices(std::vector<Vertex>& in_vertices, std::vector<unsigned int>& out_indices, std::vector<Vertex>& out_vertices) {
		std::unordered_map<Vertex, unsigned int> VertexToOutIndex;

		// For each input vertex
		for (auto& vertex : in_vertices) {
			// Try to find a similar vertex in out_XXXX
			unsigned int index;
			bool found = GetSimilarVertexIndex(vertex, VertexToOutIndex, index);

			if (found) { // A similar vertex is already in the VBO, use it instead !
				out_indices.push_back(index);
			}
			else { // If not, it needs to be added in the output data.
				out_vertices.push_back(vertex);
				unsigned short newindex = (unsigned short)out_vertices.size() - 1;
				out_indices.push_back(newindex);
				VertexToOutIndex[vertex] = newindex;
			}
		}
	}

	class StaticMesh {
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		unsigned int vertexArrayObject = -1;
		unsigned int elementBufferObject = -1;

		inline static StaticMesh* defaultQuad = nullptr;
		inline static StaticMesh* defaultQuadDouble = nullptr;
		inline static StaticMesh* defaultCube = nullptr;

		static StaticMesh* CreateDefaultQuad();

		static StaticMesh* CreateDefaultQuadDoubleSided();

		static StaticMesh* CreateDefaultCube();

		void UnloadFromGPU();
	public:
		StaticMesh(const float* vertPos, const float* texCoords, int vertCount);

		StaticMesh(std::vector<Vertex> new_vertices);

		StaticMesh(std::vector<Vertex> new_vertices, std::vector<unsigned int> new_indices);

		~StaticMesh();

		void BindMeshDataToGPU();

		void Draw() const;

		static StaticMesh* GetDefaultQuad();

		static StaticMesh* GetDefaultQuadDoubleSided();

		static StaticMesh* GetDefaultCube();


	};




}


