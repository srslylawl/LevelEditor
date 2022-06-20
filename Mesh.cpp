#include "Mesh.h"

#include <iostream>

#include "Resources.h"
#include "glad.h"

Mesh::StaticMesh* Mesh::StaticMesh::CreateDefaultQuad() {
	const std::vector temp_vertices = {
		Vertex(-0.5f, 0.5f, 0, 0.0f, 1.0f), //top left
		Vertex(0.5f, 0.5f, 0, 1.0f, 1.0f), //top right
		Vertex(-0.5f, -0.5f, 0, 0.0f, 0.0f), //bot left
		Vertex(0.5f, -0.5f, 0, 1.0f, 0.0f) //bot right
	};

	const std::vector<unsigned int> temp_indices = { 0, 3, 1, 0, 2, 3 };

	return new StaticMesh(temp_vertices, temp_indices);;
}
Mesh::StaticMesh* Mesh::StaticMesh::CreateDefaultQuadDoubleSided() {
	std::vector temp_vertices = {
		Vertex(-0.5f, 0.5f, 0, 0.0f, 1.0f), //top left
		Vertex(0.5f, 0.5f, 0, 1.0f, 1.0f), //top right
		Vertex(-0.5f, -0.5f, 0, 0.0f, 0.0f), //bot left
		Vertex(0.5f, -0.5f, 0, 1.0f, 0.0f) //bot right
	};

	const std::vector<unsigned int> temp_indices = {
		0, 3, 1, 0, 2, 3, //front
		0, 1, 3, 0, 3, 2 // back
	};

	return new StaticMesh(temp_vertices, temp_indices);
}
Mesh::StaticMesh* Mesh::StaticMesh::CreateDefaultCube() {
	const std::vector cubeVerts = {
		// back face (CCW winding)
		Vertex(0.5f, -0.5f,  0.5f,  0.0f, 0.0f), // bottom-left
		Vertex(-0.5f, -0.5f, 0.5f,  1.0f, 0.0f), // bottom-right
		Vertex(-0.5f,  0.5f, 0.5f,  1.0f, 1.0f), // top-right
		Vertex(-0.5f,  0.5f, 0.5f,  1.0f, 1.0f), // top-right
		Vertex(0.5f,  0.5f, 0.5f,  0.0f, 1.0f), // top-left
		Vertex(0.5f, -0.5f, 0.5f,  0.0f, 0.0f), // bottom-left
		// front face (CCW winding)
		Vertex(-0.5f, -0.5f,  -0.5f,  0.0f, 0.0f), // bottom-left
		Vertex(0.5f, -0.5f,  -0.5f,  1.0f, 0.0f), // bottom-right
		Vertex(0.5f,  0.5f,  -0.5f,  1.0f, 1.0f), // top-right
		Vertex(0.5f,  0.5f,  -0.5f,  1.0f, 1.0f), // top-right
		Vertex(-0.5f,  0.5f,  -0.5f,  0.0f, 1.0f), // top-left
		Vertex(-0.5f, -0.5f,  -0.5f,  0.0f, 0.0f), // bottom-left
		// left face (CCW)
		Vertex(-0.5f, -0.5f, 0.5f,  0.0f, 0.0f), // bottom-left
		Vertex(-0.5f, -0.5f,  -0.5f,  1.0f, 0.0f), // bottom-right
		Vertex(-0.5f,  0.5f,  -0.5f,  1.0f, 1.0f), // top-right
		Vertex(-0.5f,  0.5f,  -0.5f,  1.0f, 1.0f), // top-right
		Vertex(-0.5f,  0.5f, 0.5f,  0.0f, 1.0f), // top-left
		Vertex(-0.5f, -0.5f, 0.5f,  0.0f, 0.0f), // bottom-left
		// right face (CCW)
		Vertex(0.5f, -0.5f,  -0.5f,  0.0f, 0.0f), // bottom-left
		Vertex(0.5f, -0.5f, 0.5f,  1.0f, 0.0f), // bottom-right
		Vertex(0.5f,  0.5f, 0.5f,  1.0f, 1.0f), // top-right
		Vertex(0.5f,  0.5f, 0.5f,  1.0f, 1.0f), // top-right
		Vertex(0.5f,  0.5f,  -0.5f,  0.0f, 1.0f), // top-left
		Vertex(0.5f, -0.5f,  -0.5f,  0.0f, 0.0f), // bottom-left
		// bottom face (CCW)      
		Vertex(-0.5f, -0.5f, 0.5f,  0.0f, 0.0f), // bottom-left
		Vertex(0.5f, -0.5f, 0.5f,  1.0f, 0.0f), // bottom-right
		Vertex(0.5f, -0.5f,  -0.5f,  1.0f, 1.0f), // top-right
		Vertex(0.5f, -0.5f,  -0.5f,  1.0f, 1.0f), // top-right
		Vertex(-0.5f, -0.5f,  -0.5f,  0.0f, 1.0f), // top-left
		Vertex(-0.5f, -0.5f, 0.5f,  0.0f, 0.0f), // bottom-left
		// top face (CCW)
		Vertex(-0.5f,  0.5f,  -0.5f,  0.0f, 0.0f), // bottom-left
		Vertex(0.5f,  0.5f,  -0.5f,  1.0f, 0.0f), // bottom-right
		Vertex(0.5f,  0.5f, 0.5f,  1.0f, 1.0f), // top-right
		Vertex(0.5f,  0.5f, 0.5f,  1.0f, 1.0f), // top-right
		Vertex(-0.5f,  0.5f, 0.5f,  0.0f, 1.0f), // top-left
		Vertex(-0.5f,  0.5f,  -0.5f,  0.0f, 0.0f) // bottom-left
	};

	return new StaticMesh{cubeVerts};
}
void Mesh::StaticMesh::UnloadFromGPU() {
	glDeleteVertexArrays(1, &vertexArrayObject);
}
Mesh::StaticMesh::StaticMesh(const float* vertPos, const float* texCoords, int vertCount) {
	std::vector<Vertex> temp_verts;
	PackVertexData(vertPos, texCoords, vertCount, temp_verts);
	GenIndices(temp_verts, indices, vertices);

	BindMeshDataToGPU();
}
Mesh::StaticMesh::StaticMesh(std::vector<Vertex> new_vertices) {
	GenIndices(new_vertices, indices, vertices);
	BindMeshDataToGPU();
}
Mesh::StaticMesh::StaticMesh(std::vector<Vertex> new_vertices, std::vector<unsigned> new_indices):
	vertices(new_vertices),
	indices(new_indices) {

	BindMeshDataToGPU();
}

Mesh::StaticMesh::~StaticMesh() {
	UnloadFromGPU();
	std::cout << "Mesh deleted" << std::endl;
}
void Mesh::StaticMesh::BindMeshDataToGPU() {
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
void Mesh::StaticMesh::Draw() const {
	glBindVertexArray(vertexArrayObject);
	glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
Mesh::StaticMesh* Mesh::StaticMesh::GetDefaultQuad() {
	if (defaultQuad == nullptr) {
		auto quad = CreateDefaultQuad();
		Resources::Meshes.push_back(quad);
		defaultQuad = quad;
	}

	return defaultQuad;
}
Mesh::StaticMesh* Mesh::StaticMesh::GetDefaultQuadDoubleSided() {
	if (defaultQuadDouble == nullptr) {
		auto quad = CreateDefaultQuadDoubleSided();
		Resources::Meshes.push_back(quad);
		defaultQuadDouble = quad;
	}

	return defaultQuadDouble;
}
Mesh::StaticMesh* Mesh::StaticMesh::GetDefaultCube() {
	if (defaultCube == nullptr) {
		auto cube = CreateDefaultCube();
		Resources::Meshes.push_back(cube);
		defaultCube = cube;
	}

	return defaultCube;
}
