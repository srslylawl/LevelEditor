#pragma once

#include "Camera.h"
#include "Mesh.h"
#include "Shader.h"


class MainWindow {
	SDL_Window* SDLWindow = nullptr;
	SDL_GLContext gl_context = nullptr;

	int m_width;
	int m_height;
	const char* m_title = "New Window";

	unsigned int VertexArrayObject = 0;
	unsigned int elementBufferObject = 0;

	Camera* mainCamera;

	bool showDebugWindow = false;

	short renderMode = 0;

	std::unique_ptr<Shader> shaderProgramUPTR;

	std::vector<Mesh> meshes;

	void RenderImGui();
	void RenderOpenGL();

	bool InitSDL();
	bool InitOpenGL();
	bool InitDearImGui();

public:
	unsigned int currentTexture = 0;
	SDL_Window* GetSDLWindow() { return SDLWindow; }
	MainWindow(int width, int height, const char* title);
	bool Initialize();
	void Render();
	
	void OnResized(int width, int height);
	void Close();
};