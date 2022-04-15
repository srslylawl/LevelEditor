#pragma once
#include "glad.h"
#include "glm/glm.hpp"

struct Vertex {
	vec3 Position;
	vec2 TexCoords;

	Vertex(const float &pos_x, const float& pos_y, const float& pos_z, const float& uv_x, const float& uv_y) : Position(pos_x, pos_y, pos_z), TexCoords(uv_x, uv_y) {}
};

class Mesh {
	std::vector<Vertex> vertices;
	unsigned int vertexArrayObjectIndex = -1;

public:
	Mesh(float* vertPos, float* texCoords, int vertCount) {
		for (int vertex = 0; vertex < vertCount; vertex++) {
			const int currVertPos = vertex * 3;
			const int currTexPos = vertex * 2;
			vertices.emplace_back(vertPos[currVertPos],
				vertPos[currVertPos+1],
				vertPos[currVertPos+2],
				texCoords[currTexPos],
				texCoords[currTexPos+1]);
		}
		BindMeshDataToGPU();
	}

	void BindMeshDataToGPU() {
		// Create VAO to store all object data in
		glGenVertexArrays(1, &vertexArrayObjectIndex);
		glBindVertexArray(vertexArrayObjectIndex);

		//Create Buffers to store vertex data in
		GLuint VBO;
		glGenBuffers(1, &VBO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

		
		// Vertex Pos Data is VertexAttribute 0
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
		glEnableVertexAttribArray(0);

		// TexCoord Data is VertexAttribute 1
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(offsetof(Vertex, TexCoords)));
		glEnableVertexAttribArray(1);
	}

	void Draw() const {
		glBindVertexArray(vertexArrayObjectIndex);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());
	}

	~Mesh() {
		glDeleteVertexArrays(1, &vertexArrayObjectIndex);
	}
};

