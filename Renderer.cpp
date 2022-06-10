#include "Renderer.h"

#include <filesystem>
#include <iostream>
#include <SDL.h>

#include "glad.h"
#include "Resources.h"
#include "Shader.h"
#include "Time.h"

using namespace Rendering;

bool Renderer::InitOpenGL(SDL_Window* window) {
	// set OpenGL attributes
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	SDL_GL_SetAttribute(
		SDL_GL_CONTEXT_PROFILE_MASK,
		SDL_GL_CONTEXT_PROFILE_CORE
	);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);

	gl_context = SDL_GL_CreateContext(window);
	if (!gl_context) {
		std::cout << "Unable to create gl context " << SDL_GetError() << std::endl;
		return false;
	}
	SDL_GL_MakeCurrent(window, gl_context);

	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
		std::cerr << "[ERROR] Couldn't initialize glad" << std::endl;
		return false;
	}

	// set clearing color (background color)
	glClearColor(0.2f, 0.2f, 0.2f, 1);

	//enable alpha blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// enable z testing
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);

	std::vector cubeVerts = {
		// back face (CCW winding)
		Vertex(0.5f, -0.5f,  0.5f,  0.0f, 0.0f), // bottom-left
		Vertex(-0.5f, -0.5f, 0.5f,  1.0f, 0.0f), // bottom-right
		Vertex(-0.5f,  0.5f, 0.5f,  1.0f, 1.0f), // top-right
		Vertex(-0.5f,  0.5f, 0.5f,  1.0f, 1.0f), // top-right
		Vertex( 0.5f,  0.5f, 0.5f,  0.0f, 1.0f), // top-left
		Vertex( 0.5f, -0.5f, 0.5f,  0.0f, 0.0f), // bottom-left
		// front face (CCW winding)
		Vertex(-0.5f, -0.5f,  -0.5f,  0.0f, 0.0f), // bottom-left
		Vertex( 0.5f, -0.5f,  -0.5f,  1.0f, 0.0f), // bottom-right
		Vertex( 0.5f,  0.5f,  -0.5f,  1.0f, 1.0f), // top-right
		Vertex( 0.5f,  0.5f,  -0.5f,  1.0f, 1.0f), // top-right
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
		Vertex( 0.5f, -0.5f, 0.5f,  1.0f, 0.0f), // bottom-right
		Vertex( 0.5f, -0.5f,  -0.5f,  1.0f, 1.0f), // top-right
		Vertex( 0.5f, -0.5f,  -0.5f,  1.0f, 1.0f), // top-right
		Vertex(-0.5f, -0.5f,  -0.5f,  0.0f, 1.0f), // top-left
		Vertex(-0.5f, -0.5f, 0.5f,  0.0f, 0.0f), // bottom-left
		// top face (CCW)
		Vertex(-0.5f,  0.5f,  -0.5f,  0.0f, 0.0f), // bottom-left
		Vertex( 0.5f,  0.5f,  -0.5f,  1.0f, 0.0f), // bottom-right
		Vertex( 0.5f,  0.5f, 0.5f,  1.0f, 1.0f), // top-right
		Vertex( 0.5f,  0.5f, 0.5f,  1.0f, 1.0f), // top-right
		Vertex(-0.5f,  0.5f, 0.5f,  0.0f, 1.0f), // top-left
		Vertex(-0.5f,  0.5f,  -0.5f,  0.0f, 0.0f) // bottom-left
	};
	Resources::Meshes.emplace_back(cubeVerts);
	auto quad = Mesh::StaticMesh::DefaultQuad();
	Resources::Meshes.push_back(quad);
	//auto quadDS = Mesh::StaticMesh::DefaultQuadDoubleSided();
	//Resources::Meshes.push_back(quadDS);

	//init shaders
	CompileShader();
	

	int width, height;
	mainWindow->GetSize(width, height);
	camera = new Camera(width, height, true);

	return true;
}

void Renderer::CompileShader() {
	if (defaultShader != nullptr) delete defaultShader;
	if (gridShader != nullptr) delete gridShader;


	//const std::filesystem::path vertShaderPath = std::filesystem::current_path().append("Shaders/defaultVertShader.vert");
	//const std::filesystem::path fragShaderPath = std::filesystem::current_path().append("Shaders/defaultFragShader.frag");

	//defaultShader = new Shader(vertShaderPath.string().c_str(), fragShaderPath.string().c_str());

	//gridShader = new Shader(std::filesystem::current_path().append("Shaders/gridVertShader.vert").string().c_str(), 
	//	std::filesystem::current_path().append("Shaders/gridFragShader.frag").string().c_str());

	defaultShader = new Shader("default");
	gridShader = new Shader("2DGrid");
}

bool Renderer::Init(MainWindow* window) {
	mainWindow = window;

	return InitOpenGL(window->GetSDLWindow());
}

void Renderer::Render() {
	//___ LOOPED RENDERING CODE
	// use shader program
	defaultShader->Use();
	//glUniform1i(glGetUniformLocation(shaderProgramUPTR->ID, "texture1"), 0); // -- redundant?

	// view matrix transforms world space to view (camera) space
	defaultShader->setMat4("view", *Camera::Main->GetViewMatrix());

	// projection matrix transforms view space to however we want to display (orthogonal, perspective)
	defaultShader->setMat4("projection", *Camera::Main->GetProjectionMatrix());

	glActiveTexture(GL_TEXTURE0);

	if (Resources::Textures.begin() != Resources::Textures.end()) {
		glBindTexture(GL_TEXTURE_2D, Resources::Textures.begin()->second.ID);
	}

	const vec3 cubePositions[] = {
	vec3(0.0f,  0.0f,  0.0f),
	vec3(2.0f,  2.0f, 2.0f),
	vec3(-2.0f, -2.0f, -2.0f)
	};


	for (int i = 0; i < 3; i++) {
		auto pos = cubePositions[i];
		// model matrix transforms coords to world space
		auto xPos = sin(Time::GetTime()) * 3.0f;
		auto yPos = cos(Time::GetTime()) * 3.0f;
		if(i == 0) {
			xPos = -xPos;
		}

		if(i == 2) {
			yPos = -yPos;
		}
		pos.x += xPos;
		pos.y += yPos;
		mat4 modelM = translate(mat4(1.0f), pos);

		defaultShader->setMat4("model", modelM);

		Resources::Meshes[0].Draw();
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	auto mousePos = Input::GetMousePosition();
	auto mouseGridPos = camera->ScreenToGridPosition(mousePos.x, mousePos.y);

	gridShader->Use();
	gridShader->setMat4("view", *Camera::Main->GetViewMatrix());
	gridShader->setMat4("projection", *Camera::Main->GetProjectionMatrix());
	gridShader->setVec("mousePos", mouseGridPos);
	gridShader->setMat4("model", scale(mat4(1.0f), vec3(1000)));
	Resources::Meshes[1].Draw();

	//unbind vertex array
	glBindVertexArray(0);
}


