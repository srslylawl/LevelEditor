#pragma once

#include "Camera.h"
#include "Mesh.h"
#include "Shader.h"

namespace Rendering {
	
class MainWindow {
	SDL_Window* SDLWindow = nullptr;

	static int width;
	static int height;
	const char* m_title = "New Window";

	bool showDebugWindow = false;

	short renderMode = 0;

	std::unique_ptr<Shader> shaderProgramUPTR;

	std::vector<Mesh::StaticMesh> meshes;

	void RenderImGui();
	bool InitSDL();
	bool InitDearImGui();


public:
	SDL_Window* GetSDLWindow() { return SDLWindow; }
	MainWindow(int width, int height, const char* title);
	bool Initialize();
	void Render();
	
	void OnResized(int width, int height);
	void Close();

	static void GetSize(int& out_width, int& out_height) {
		out_width = width;
		out_height = height;
	}
};
}
