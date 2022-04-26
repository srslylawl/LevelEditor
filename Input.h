#pragma once

#include <combaseapi.h>
#include <functional>
#include <map>
#include <SDL.h>
#include <unordered_set>
#include <iostream>
#include <set>
#include <utility>


enum class KeyEvent {
	KeyDown = 1 << 0,
	KeyHold = 1 << 1,
	KeyUp = 1 << 2,
};

enum class MouseButton {
	Left	= 1 << 0,
	Right	= 1 << 1,
	Middle	= 1 << 2,
	X1		= 1 << 3,
	X2		= 1 << 4,
};

struct InputKeyBinding {
	std::function<void(KeyEvent)> Action;
	GUID BindingGUID;
	SDL_Keycode Keycode;
	InputKeyBinding(const GUID guid, std::function<void(KeyEvent)> action, SDL_Keycode keycode) : Action(std::move(action)), BindingGUID(guid), Keycode(keycode) { }
};

struct MouseMotion {
	int posX;
	int posY;
	int deltaX;
	int deltaY;
};

class InputMouseEvent {
	const std::map<MouseButton, std::unordered_set<KeyEvent>>* keyEvents = nullptr;
public:
	const MouseMotion* motion;

	bool GetMouseKey(KeyEvent mouse_key_event, MouseButton button) const {
		const auto iterator = keyEvents->find(button);
		if (iterator == keyEvents->end()) return false;
		return iterator->second.find(mouse_key_event) != iterator->second.end();
	}

	bool GetMouseKeyDown (MouseButton button) const {
		return GetMouseKey(KeyEvent::KeyDown, button);
	}

	bool GetMouseKeyUp(MouseButton button) const {
		return GetMouseKey(KeyEvent::KeyUp, button);
	}

	bool GetMouseKeyHold(MouseButton button) const {
		return GetMouseKey(KeyEvent::KeyHold, button);
	}

	InputMouseEvent(const std::map<MouseButton, std::unordered_set<KeyEvent>>* key_events, const MouseMotion* motion)
		: keyEvents(key_events),
		  motion(motion) {}
};


struct InputMouseBinding {
	std::function<void(const InputMouseEvent*)> Action;
	GUID BindingGUID;

	InputMouseBinding(std::function<void(const InputMouseEvent*)> action, GUID guid) : Action(std::move(action)), BindingGUID(guid) {}
};

/**
 * \brief Handles Input and delegates it to subscribers
 */
class Input {
	Input() = default;
	inline static std::map<SDL_Keycode, std::unordered_set<InputKeyBinding*>> keyBindings;
	inline static std::unordered_set<InputMouseBinding*> mouseBindings;
	inline static std::map<SDL_Keycode, bool> keysHeldDown;
	inline static std::set<SDL_Keycode> keysPressedThisFrame;
	inline static std::map<MouseButton, std::unordered_set<KeyEvent>> mouseKeyEvents;
	inline static MouseMotion mouseMotion;
	inline static std::pair<int, int> savedMousePosition;
public:
	static InputKeyBinding* AddKeyBinding(const SDL_Keycode keycode, std::function<void(KeyEvent)> action) {
		GUID newGUID;
		CoCreateGuid(&newGUID);
		auto* newBinding = new InputKeyBinding(newGUID, action, keycode);
		keyBindings[keycode].insert(newBinding);
		return newBinding;
	}
	static void RemoveKeyBinding(InputKeyBinding* binding) {
		if (!keyBindings[binding->Keycode].erase(binding)) {
			std::cout << "Failed to remove binding for keycode: " << binding->Keycode << std::endl;
		}
		delete binding;
	}

	static void ReceiveKeyDownInput(const SDL_Keycode& keycode) {
		if (keysHeldDown[keycode]) return; //don't care if its already held down

		keysHeldDown[keycode] = true;

		keysPressedThisFrame.insert(keycode);

		for (auto const& binding : keyBindings[keycode]) {
			binding->Action(KeyEvent::KeyDown);
		}
	}

	static void ReceiveKeyUpInput(const SDL_Keycode& keycode) {
		if (!keysHeldDown[keycode]) {
			std::cout << "Key " << keycode << " release received, but was not held down?" << std::endl;
			return;
		}
		keysHeldDown.erase(keycode);

		for (auto const& binding : keyBindings[keycode]) {
			binding->Action(KeyEvent::KeyUp);
		}
	}

	static InputMouseBinding* AddMouseBinding(const std::function<void(const InputMouseEvent*)>& action) {
		GUID newGuid;
		CoCreateGuid(&newGuid);
		const auto binding = new InputMouseBinding(action, newGuid);

		mouseBindings.insert(binding);

		return binding;
	}

	static void RemoveMouseBinding(InputMouseBinding* binding) {
		if (!mouseBindings.erase(binding)) {
			std::cout << "Failed to remove mouseBinding" << std::endl;
		}

		delete binding;
	}

	static void ReceiveMouseMotion(const SDL_MouseMotionEvent& e) {
		mouseMotion.deltaX = e.xrel;
		mouseMotion.deltaY = e.yrel;
		mouseMotion.posX = e.x;
		mouseMotion.posY = e.y;

		//printf("Mousemotion: x:%i,y:%i, relx:%i,rely:%i\n", mouseMotion.posY, mouseMotion.posY, mouseMotion.deltaX, mouseMotion.deltaY);
	}

	static void ReceiveMouseButtonEvent(const SDL_MouseButtonEvent e) {
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
			break;
		case SDL_BUTTON_X2:
			button = MouseButton::X2;
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

	static void DelegateInputActions() {
		//Key binds
		for (auto const& [keycode, beingHeld] : keysHeldDown) {
			for (auto const& binding : keyBindings[keycode]) {
				binding->Action(KeyEvent::KeyHold);
			}
		}
		keysPressedThisFrame.clear();

		// mouse binds
		const auto mouseEvent = InputMouseEvent(&mouseKeyEvents, &mouseMotion);
		for (auto const& mouse_binding : mouseBindings) {
			mouse_binding->Action(&mouseEvent);
		}
		// clear mouse key events - remove all if keyup, remove only keydown if its present
		for (auto &[key, e] : mouseKeyEvents) {
			if (e.count(KeyEvent::KeyUp)) {
				e.clear();
				continue;
			}

			if (e.count(KeyEvent::KeyDown))
				e.erase(KeyEvent::KeyDown);
		}
		// clear mouse motion
		mouseMotion = {};
	}

	static void SetMouseCapture(bool set) {
		if(set) {
			SDL_GetGlobalMouseState(&savedMousePosition.first, &savedMousePosition.second);
			//std::cout << "Saved mousePos: x:" << savedMousePosition.first << ", y:" << savedMousePosition.second << std::endl;
		}
		SDL_SetRelativeMouseMode(static_cast<SDL_bool>(set));

		if(!set) {
			SDL_WarpMouseGlobal(savedMousePosition.first, savedMousePosition.second);
			SDL_GetGlobalMouseState(&savedMousePosition.first, &savedMousePosition.second);
			//std::cout << "mouse warped to X:" << savedMousePosition.first << ", y:" << savedMousePosition.second << std::endl;
		}
	}


	static void Cleanup() {
		for (auto& [key, value] : keyBindings) {
			Cleanup(value);
		}

		Cleanup(mouseBindings);
	}

	template<typename T>
	static void Cleanup(T collection) {
		if (collection.size() > 0) {
			std::cout << "Input wasn't properly unsubscribed - remaining entries: " << collection.size() << std::endl;

			for (auto ptr : collection) {
				delete ptr;
			}
		}
	}
};




