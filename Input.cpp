#include "Input.h"
#include "MainWindow.h"

glm::vec2 Input::GetMousePosition() {
	int x = 0;
	int y = 0;
	SDL_GetMouseState(&x, &y);
	int _, yScreen;
	Rendering::MainWindow::GetSize(_, yScreen);
	//Y is 0 at top in SDL, so set it to 0 at bottom by subtracting it from screen height in order to make it more intuitive
	y = yScreen - y;

	return { x, y };
}
