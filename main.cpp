#include <SDL.h>
#include "MainWindow.h"
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include "Input.h"
#include "Resources.h"
#include "Time.h"

void HandleSDLEvents(SDL_Event& sdlEvent, bool& quit);

int main(int arg, char** args) {
	//init time module
	Time::Init();
	{
		//Create SDL Window
		Rendering::MainWindow mainWindow = Rendering::MainWindow(1200, 800, "LevelEditor");
		if (!mainWindow.Initialize()) {
			printf("Failed to init main window");
			return 1;
		}

		ImGuiIO& imgui_io = ImGui::GetIO();
		SDL_Event sdlEvent;
		bool quit = false;
		while (!quit) {
			Time::CalcDeltaTime();
			HandleSDLEvents(sdlEvent, quit);

			if (!imgui_io.WantCaptureMouse) {
				Input::DelegateMouseActions();
			}
			else {
				Input::ClearMouseActions();
			}

			if (!imgui_io.WantCaptureKeyboard) {
				Input::DelegateKeyboardActions();
			}
			else {
				Input::ClearKeyboardActions();
			}
			mainWindow.Render();
		}
		mainWindow.Close();
	}

	Input::Cleanup();
	Resources::FreeAll();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
	//Quit SDL subsystems
	SDL_Quit();

	return 0;
}

void HandleSDLEvents(SDL_Event& sdlEvent, bool& quit) {
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
				Rendering::MainWindow::OnResized(w, h);
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

		case SDL_MOUSEWHEEL:
			Input::ReceiveMouseWheelEvent(sdlEvent.wheel);
		}
	}
}

