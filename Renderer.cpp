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
	//glEnable(GL_DEPTH_TEST);

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

	// view matrix transforms world space to view (camera) space
	defaultShader->setMat4("view", *Camera::Main->GetViewMatrix());

	// projection matrix transforms view space to however we want to display (orthogonal, perspective)
	defaultShader->setMat4("projection", *Camera::Main->GetProjectionMatrix());

	for (const auto& renderObject : RenderObjects) {
		if (!renderObject->renderingEnabled) continue;
		renderObject->Render();
	}

	glActiveTexture(GL_TEXTURE0); // next commands affect texture slot 0
	glBindTexture(GL_TEXTURE_2D, 0); // bind texture with id 0 (none) to active texture slot
	
	glBindVertexArray(0); //unbind vertex array
}


