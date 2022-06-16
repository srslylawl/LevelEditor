#pragma once
#include "Camera.h"
#include "GridToolBar.h"
#include "TileMap.h"

namespace Rendering {
	
class MainWindow {
	SDL_Window* SDLWindow = nullptr;

	static int width;
	static int height;
	const char* m_title = "New Window";
	bool showDebugWindow = false;
	short renderMode = 0;

	InputMouseBinding* binding = nullptr;

	GridTools::GridToolBar* gridToolBar = nullptr;

	void RenderImGui();
	bool InitSDL();
	bool InitDearImGui();

public:
	SDL_Window* GetSDLWindow() const { return SDLWindow; }
	MainWindow(int new_width, int new_height, const char* title);
	void OnMouseInput(const InputMouseEvent* event);
	bool Initialize();
	void Render();

	Tiles::TileMap* tileMap = nullptr;
	
	void OnResized(int width, int height);
	void Close();

	static void GetSize(int& out_width, int& out_height) {
		out_width = width;
		out_height = height;
	}
};
}
