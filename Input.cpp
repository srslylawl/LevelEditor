#include "Input.h"

#include <iostream>

#include "MainWindow.h"

Input::Input() = default;

InputKeyBinding* Input::AddKeyBinding(const SDL_Keycode keycode, std::function<void(KeyEvent)> action) {
	auto* newBinding = new InputKeyBinding(action, keycode);
	keyBindings[keycode].insert(newBinding);
	return newBinding;
}

void Input::RemoveKeyBinding(InputKeyBinding* binding) {
	if (!keyBindings[binding->Keycode].erase(binding)) {
		std::cout << "Failed to remove binding for keycode: " << binding->Keycode << std::endl;
	}
	delete binding;
}

void Input::ReceiveKeyDownInput(const SDL_Keycode& keycode) {
	if (keysHeldDown[keycode]) return; //don't care if its already held down
	keysHeldDown[keycode] = true;
	keysDownThisFrame.insert(keycode);
}

void Input::ReceiveKeyUpInput(const SDL_Keycode& keycode) {
	if (!keysHeldDown[keycode]) {
		std::cout << "Key " << keycode << " release received, but was not held down?" << std::endl;
		return;
	}
	keysHeldDown.erase(keycode);
	keysUpThisFrame.insert(keycode);
}

InputMouseBinding* Input::AddMouseBinding(const std::function<void(const InputMouseEvent*)>& action) {
	const auto binding = new InputMouseBinding(action);

	mouseBindings.insert(binding);

	return binding;
}

void Input::RemoveMouseBinding(InputMouseBinding* binding) {
	if (!mouseBindings.erase(binding)) {
		std::cout << "Failed to remove mouseBinding" << std::endl;
	}

	delete binding;
	binding = nullptr;
}

void Input::ReceiveMouseMotion(const SDL_MouseMotionEvent& e) {
	// only setting relative here as motion should always contain actual pos which would be missing when no motion occurs
	// making sure to add to delta, as multiple updates may happen per frame
	mouseMotion.deltaX += e.xrel;
	mouseMotion.deltaY += e.yrel;
}

void Input::ReceiveMouseButtonEvent(const SDL_MouseButtonEvent e) {
	const bool pressed = e.state == SDL_PRESSED;
	MouseButton button;
	switch (e.button) {
	case SDL_BUTTON_LEFT:
		button = MouseButton::Left;
		break;
	case SDL_BUTTON_RIGHT:
		button = MouseButton::Right;
		break;
	case SDL_BUTTON_MIDDLE:
		button = MouseButton::Middle;
		break;
	case SDL_BUTTON_X1:
		button = MouseButton::X1;
		std::cout << "Mouse Button X1" << e.button << std::endl;
		break;
	case SDL_BUTTON_X2:
		button = MouseButton::X2;
		std::cout << "Mouse Button X2" << e.button << std::endl;
		break;
	default:
		std::cout << "Unknown mouse button event: " << e.button << std::endl;
	}
	auto& keyEvents = mouseKeyEvents[button];
	// key down
	if (pressed) {
		if (keyEvents.count(KeyEvent::KeyHold)) return; // if button is already held, we choose to ignore additional press events

		keyEvents.insert(KeyEvent::KeyHold);
		keyEvents.insert(KeyEvent::KeyDown);

		return;
	}

	// key up
	keyEvents.clear(); // remove hold and down events
	keyEvents.insert(KeyEvent::KeyUp);
}

void Input::ReceiveMouseWheelEvent(const SDL_MouseWheelEvent e) {
	MouseWheel wheel;
	wheel.scrollDelta = e.y;
	wheel.scrollDeltaPrecise = e.preciseY;

	mouseWheel = wheel;
}

void Input::DelegateMouseActions() {
	// make sure total mouse pos is always accurate
	GetMousePosition(mouseMotion.posX, mouseMotion.posY);
	const auto mouseEvent = InputMouseEvent(&mouseKeyEvents, &mouseMotion, &mouseWheel);
	for (auto const& mouse_binding : mouseBindings) {
		mouse_binding->Action(&mouseEvent);
	}
	// clear mouse key events - remove all if keyup, remove only keydown if its present
	for (auto& [key, e] : mouseKeyEvents) {
		if (e.count(KeyEvent::KeyUp)) {
			e.clear();
			continue;
		}

		if (e.count(KeyEvent::KeyDown))
			e.erase(KeyEvent::KeyDown);
	}
	ClearMouseActions();
}

void Input::DelegateKeyboardActions() {
	// Pressed Keys
	for (auto const& key : keysDownThisFrame) {
		for (auto const& binding : keyBindings[key]) {
			binding->Action(KeyEvent::KeyDown);
		}
	}

	// Held Keys
	for (auto const& [keycode, beingHeld] : keysHeldDown) {
		for (auto const& binding : keyBindings[keycode]) {
			binding->Action(KeyEvent::KeyHold);
		}
	}

	// Released Keys
	for (auto const& key : keysUpThisFrame) {
		for (auto const& binding : keyBindings[key]) {
			binding->Action(KeyEvent::KeyUp);
		}
	}

	ClearKeyboardActions();
}

void Input::ClearMouseActions() {
	// clear mouse motion and wheel
	mouseMotion = {};
	mouseWheel = {};
}

void Input::ClearKeyboardActions() {
	keysDownThisFrame.clear();
	keysUpThisFrame.clear();
}

void Input::SetMouseCapture(bool set) {
	if (set) {
		SDL_GetGlobalMouseState(&savedMousePosition.first, &savedMousePosition.second);
		//std::cout << "Saved mousePos: x:" << savedMousePosition.first << ", y:" << savedMousePosition.second << std::endl;
	}
	SDL_SetRelativeMouseMode(static_cast<SDL_bool>(set));

	if (!set) {
		SDL_WarpMouseGlobal(savedMousePosition.first, savedMousePosition.second);
		SDL_GetGlobalMouseState(&savedMousePosition.first, &savedMousePosition.second);
		//std::cout << "mouse warped to X:" << savedMousePosition.first << ", y:" << savedMousePosition.second << std::endl;
	}
}

void Input::Cleanup() {
	for (auto& [key, value] : keyBindings) {
		Cleanup(value);
	}

	Cleanup(mouseBindings);
}

template <typename T>
void Input::Cleanup(T collection) {
	if (collection.size() > 0) {
		std::cout << "Input wasn't properly unsubscribed - remaining entries: " << collection.size() << std::endl;

		for (auto ptr : collection) {
			delete ptr;
		}
	}
}

glm::vec2 Input::GetMousePosition() {
	int x = 0; int y = 0;
	GetMousePosition(x, y);
	return { x, y };
}

void Input::GetMousePosition(int &x, int &y) {
	SDL_GetMouseState(&x, &y);
	int _, yScreen;
	Rendering::MainWindow::GetSize(_, yScreen);
	//Y is 0 at top in SDL, so set it to 0 at bottom by subtracting it from screen height in order to make it more intuitive
	y = yScreen - y;
}
