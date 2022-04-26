#pragma once

#include <unordered_map>
#include <utility>
#include <vector>
#include <xhash>

#include "glad.h"
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

	public:
		StaticMesh(const float* vertPos, const float* texCoords, int vertCount) {
			std::vector<Vertex> temp_verts;
			PackVertexData(vertPos, texCoords, vertCount, temp_verts);
			GenIndices(temp_verts, indices, vertices);

			BindMeshDataToGPU();
		}
		StaticMesh(std::vector<Vertex> new_vertices) {
			GenIndices(new_vertices, indices, vertices);
			BindMeshDataToGPU();
		}
		StaticMesh(std::vector<Vertex> new_vertices, std::vector<unsigned int> new_indices) :
			vertices(new_vertices),
			indices(new_indices) {

			BindMeshDataToGPU();
		}

		void BindMeshDataToGPU() {
			// Create VAO to store all object data in
			glGenVertexArrays(1, &vertexArrayObject);
			glBindVertexArray(vertexArrayObject);

			//Create Buffers to store vertex data in
			GLuint VBO;
			glGenBuffers(1, &VBO);
			glGenBuffers(1, &elementBufferObject);

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferObject);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

			// Vertex Pos Data is VertexAttribute 0
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
			glEnableVertexAttribArray(0);

			// TexCoord Data is VertexAttribute 1
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, TexCoords)));
			glEnableVertexAttribArray(1);

			glBindVertexArray(0);
		}

		void Draw() const {
			glBindVertexArray(vertexArrayObject);
			glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}

		static StaticMesh DefaultQuad() {
			std::vector temp_vertices = {
				Vertex(-0.5f, 0.5f, 0, 0.0f, 1.0f), //top left
				Vertex(0.5f, 0.5f, 0, 1.0f, 1.0f), //top right
				Vertex(-0.5f, -0.5f, 0, 0.0f, 0.0f), //bot left
				Vertex(0.5f, -0.5f, 0, 1.0f, 0.0f) //bot right
			};

			std::vector<unsigned int> temp_indices = { 0, 3, 1, 0, 2, 3 };

			auto quad = StaticMesh(temp_vertices, temp_indices);

			return quad;
		}

		void UnloadFromGPU() {
			glDeleteVertexArrays(1, &vertexArrayObject);
		}
	};




}


