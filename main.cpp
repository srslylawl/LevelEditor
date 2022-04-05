#include <SDL.h>
#include <cstdio>
#include "MainWindow.h"
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include <iostream>

#include "Time.h"


int main(int arg, char* args[]) {
	//init time module
	Time::Init();

	//Create SDL Window
	MainWindow mainWindow = MainWindow(800, 600, "LevelEditor");
	if (!mainWindow.Initialize()) {
		printf("Failed to init main window");
		return 1;
	}

	SDL_Event sdlEvent;
	bool quit = false;
	while (!quit) {
		Time::CalcDeltaTime();
		//std::cout << "MS: " << Time::GetDeltaTimeMS() << endl;
		//std::cout << "FPS: " << (1 / Time::GetDeltaTimeMS() * 1000) << endl;
		while (SDL_PollEvent(&sdlEvent) != 0) {
			ImGui_ImplSDL2_ProcessEvent(&sdlEvent);
			switch (sdlEvent.type) {
			case SDL_QUIT:
				quit = true;
				break;

			case SDL_WINDOWEVENT:
				
				switch (sdlEvent.window.event) {
				case SDL_WINDOWEVENT_RESIZED:
					const int w = sdlEvent.window.data1;
					const int h = sdlEvent.window.data2;
					mainWindow.OnResized(w, h);
					break;
				}
				break;

			case SDL_KEYDOWN:
				switch (sdlEvent.key.keysym.sym) {
				case SDLK_ESCAPE:
					//quit = true;
					break;
				}
				break;
			}
		}

		mainWindow.Render();
	}
	mainWindow.Close();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
	//Quit SDL subsystems
	SDL_Quit();

	return 0;
}
