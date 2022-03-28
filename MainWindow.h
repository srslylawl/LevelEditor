#pragma once


class MainWindow {
private:
	SDL_Window* SDLWindow = nullptr;

	int m_width = 800;
	int m_height = 600;
	const char* m_title = "New Window";

	void RenderImGui();

public:
	SDL_Window* GetSDLWindow() { return SDLWindow; }
	MainWindow(int width, int height, const char* title);
	bool Initialize();
	void Render();
	
	void OnResized(int height, int width);
	void Close();
};