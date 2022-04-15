#include <SDL.h>
#include <cstdio>
#include "MainWindow.h"
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include "Input.h"

#include "Time.h"

int main(int arg, char* args[]) {
	//init time module
	Time::Init();
	{
		//Create SDL Window
		MainWindow mainWindow = MainWindow(1200, 800, "LevelEditor");
		if (!mainWindow.Initialize()) {
			printf("Failed to init main window");
			return 1;
		}

		SDL_Event sdlEvent;
		bool quit = false;
		while (!quit) {
			Time::CalcDeltaTime();
			while (SDL_PollEvent(&sdlEvent)) {
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
					Input::ReceiveKeyDownInput(sdlEvent.key.keysym.sym);
					break;

				case SDL_KEYUP:
					Input::ReceiveKeyUpInput(sdlEvent.key.keysym.sym);
					break;

				case SDL_MOUSEMOTION:
					Input::ReceiveMouseMotion(sdlEvent.motion);
					break;

				case SDL_MOUSEBUTTONUP:
				case SDL_MOUSEBUTTONDOWN:
					Input::ReceiveMouseButtonEvent(sdlEvent.button);
					break;
				}
			}
			Input::DelegateInputActions();
			mainWindow.Render();
		}
		mainWindow.Close();
	}

	Input::Cleanup();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
	//Quit SDL subsystems
	SDL_Quit();

	return 0;
}
