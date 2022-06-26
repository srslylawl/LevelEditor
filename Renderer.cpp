#include "Renderer.h"

#include <iostream>
#include <SDL.h>

#include "Camera.h"
#include "glad.h"
#include "MainWindow.h"
#include "Mesh.h"
#include "Resources.h"
#include "Shader.h"
#include "Time.h"
#include "Renderable.h"
#include "Texture.h"

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

	//init shaders
	CompileShader();

	int width, height;
	MainWindow::GetSize(width, height);
	camera = new Camera(width, height, true);

	return true;
}

void Renderer::CompileShader() {
	delete defaultShader;
	delete gridShader;

	defaultShader = new Shader("default");
	gridShader = new Shader("2DGrid");
}

void Renderer::Exit() {
	delete defaultShader;
	delete gridShader;
	delete camera;
}

bool Renderer::Init() {
	return InitOpenGL(MainWindow::GetSDLWindow());
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

	for (const auto & render_object : RenderObjects)
		render_object->Render();

	glActiveTexture(GL_TEXTURE0);

	/* Render Cubes
	
	if (Resources::GetTextures().begin() != Resources::GetTextures().end()) {
		glBindTexture(GL_TEXTURE_2D, Resources::Textures.begin()->second->GetTextureID());
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

		Mesh::StaticMesh::GetDefaultCube()->Draw();
	}

	*/

	glBindTexture(GL_TEXTURE_2D, 0);

	// Draw 2D Grid
	// clear depth buffer to always draw grid on top
	glClear(GL_DEPTH_BUFFER_BIT);
	auto mousePos = Input::GetMousePosition();
	auto mouseGridPos = camera->ScreenToGridPosition(mousePos.x, mousePos.y);

	gridShader->Use();
	gridShader->setMat4("view", *Camera::Main->GetViewMatrix());
	gridShader->setMat4("projection", *Camera::Main->GetProjectionMatrix());
	gridShader->setVec("mousePos", mouseGridPos);
	gridShader->setMat4("model", scale(mat4(1.0f), vec3(1000)));
	Mesh::StaticMesh::GetDefaultQuad()->Draw();

	//unbind vertex array
	glBindVertexArray(0);
}


