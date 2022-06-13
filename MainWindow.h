#pragma once

#include "Camera.h"
#include "Mesh.h"
#include "Shader.h"
#include "TileMap.h"

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

	InputMouseBinding* binding = nullptr;

	void RenderImGui();
	bool InitSDL();
	bool InitDearImGui();


public:
	SDL_Window* GetSDLWindow() { return SDLWindow; }
	MainWindow(int width, int height, const char* title);
	void OnMouseDown(const InputMouseEvent* event);
	bool Initialize();
	void Render();

	Tiles::TileMap* tileData = nullptr;
	
	void OnResized(int width, int height);
	void Close();

	static void GetSize(int& out_width, int& out_height) {
		out_width = width;
		out_height = height;
	}
};
}
