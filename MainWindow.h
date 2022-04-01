#pragma once

#include "Shader.h"


class MainWindow {
private:
	SDL_Window* SDLWindow = nullptr;
	SDL_GLContext gl_context = nullptr;

	int m_width = 800;
	int m_height = 600;
	const char* m_title = "New Window";

	unsigned int VertexArrayObject;
	unsigned int elementBufferObject;

	short renderMode = 0;

	std::unique_ptr<Shader> shaderProgramUPTR;

	void RenderImGui();
	bool RenderOpenGL();

	bool InitSDL();
	bool InitOpenGL();
	bool InitDearImGui();

public:
	SDL_Window* GetSDLWindow() { return SDLWindow; }
	MainWindow(int width, int height, const char* title);
	bool Initialize();
	void Render();
	
	void OnResized(int height, int width);
	void Close();
};